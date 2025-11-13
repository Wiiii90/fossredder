#include "pch.h"
#include "utils/StatementParser.h"
#include "services/tesseract/TesseractDTO.h"
#include "models/layout/Transaction.h"
#include <regex>
#include <map>
#include <set>
#include <iostream>

namespace utils {

// Heuristic: detect header rows near top of table. Returns set of row indices considered header.
static std::set<int> detectHeaderRows(const OcrTableDto& tbl) {
    std::set<int> headers;
    if (tbl.rows <= 0) return headers;

    std::regex digitRe("\\d");
    for (int r = 0; r < tbl.rows; ++r) {
        int letters = 0, digits = 0, others = 0, tokens = 0;
        for (const auto& c : tbl.cells) {
            if (c.row != r) continue;
            std::string t = c.text;
            if (t.empty()) continue;
            ++tokens;
            for (char ch : t) {
                if (std::isalpha(static_cast<unsigned char>(ch))) ++letters;
                else if (std::isdigit(static_cast<unsigned char>(ch))) ++digits;
                else ++others;
            }
        }
        if (tokens == 0) continue;
        double alphaRatio = static_cast<double>(letters) / static_cast<double>(letters + digits + others + 1);
        double digitRatio = static_cast<double>(digits) / static_cast<double>(letters + digits + others + 1);
        // Heuristic: header rows tend to have high alpha ratio and low digit ratio
        if (alphaRatio > 0.5 && digitRatio < 0.3) {
            headers.insert(r);
        }
        // Also mark very short rows at top (r==0) as header if any text exists
        else if (r == 0 && tokens > 0 && alphaRatio > 0.25) {
            headers.insert(r);
        }
    }
    // If multiple consecutive header rows from top, keep them; otherwise limit to top 2 rows
    // (already handled by heuristics)
    return headers;
}

// Heuristic: detect which column most likely contains monetary amounts.
static int detectAmountColumn(const OcrTableDto& tbl) {
    if (tbl.cols <= 0) return -1;
    std::regex amountRe(R"((([\d\.\s]+,[0-9]{2})(\s|$)))"); // german style 1.234,56 or 1234,56
    std::vector<int> counts(tbl.cols, 0);
    for (const auto& c : tbl.cells) {
        if (c.text.empty()) continue;
        int col = c.col;
        if (col < 0 || col >= tbl.cols) continue;
        if (std::regex_search(c.text, amountRe)) counts[col]++;
        // also count parentheses-style negatives or values with currency symbol
        else if (c.text.find(',') != std::string::npos && c.text.find_first_of("0123456789") != std::string::npos) counts[col] += 0; // no increment
    }
    // pick column with max count; tie-break to rightmost
    int maxCount = 0; int bestCol = -1;
    for (int col = 0; col < tbl.cols; ++col) {
        if (counts[col] > maxCount || (counts[col] == maxCount && col > bestCol)) {
            maxCount = counts[col]; bestCol = col;
        }
    }
    // if none matched, choose rightmost non-empty column
    if (bestCol == -1 || maxCount == 0) {
        for (int col = tbl.cols - 1; col >= 0; --col) {
            bool any = false;
            for (const auto& c : tbl.cells) if (c.col == col && !c.text.empty()) { any = true; break; }
            if (any) return col;
        }
        return -1;
    }
    return bestCol;
}

// Heuristic: detect which column(s) contain transaction details (likely left columns excluding amount)
static std::vector<int> detectDetailColumns(const OcrTableDto& tbl, int amountCol) {
    std::vector<int> cols;
    if (tbl.cols <= 0) return cols;
    std::set<int> present;
    for (const auto& c : tbl.cells) if (!c.text.empty()) present.insert(c.col);
    for (int col = 0; col < tbl.cols; ++col) {
        if ((int)col == amountCol) continue;
        if (present.count(col)) cols.push_back(col);
    }
    if (cols.empty() && amountCol >= 0) {
        // fallback: everything except amount is detail; if none, take leftmost
        for (int col = 0; col < tbl.cols; ++col) if (col != amountCol) cols.push_back(col);
    }
    if (cols.empty() && tbl.cols > 0) cols.push_back(0);
    return cols;
}

// Utility: find date token in a string (dd.mm.yyyy or dd.mm)
static std::string extractDate(const std::string& s) {
    std::regex dateRe(R"((\b\d{2}\.\d{2}\.\d{4}\b)|(\b\d{2}\.\d{2}\b))");
    std::smatch m;
    if (std::regex_search(s, m, dateRe)) return m.str(0);
    return std::string();
}

// Small helper to parse amount-like strings to double. Accepts common formats like "1.234,56" or "1234.56".
static double parseAmountLike(const std::string& text) {
    if (text.empty()) return 0.0;
    std::string s;
    s.reserve(text.size());
    for (char c : text) {
        if ((c >= '0' && c <= '9') || c == ',' || c == '.' || c == '+' || c == '-') s.push_back(c);
        else if (std::isspace(static_cast<unsigned char>(c))) s.push_back(' ');
    }
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        bool hasComma = token.find(',') != std::string::npos;
        bool hasDot = token.find('.') != std::string::npos;
        std::string num = token;
        if (hasComma && !hasDot) {
            for (char &c : num) if (c == ',') c = '.';
        } else if (hasComma && hasDot) {
            std::string tmp;
            tmp.reserve(num.size());
            for (char c : num) if (c != '.') tmp.push_back(c);
            num = std::move(tmp);
            for (char &c : num) if (c == ',') c = '.';
        }
        size_t start = 0; while (start < num.size() && !(std::isdigit((unsigned char)num[start]) || num[start] == '+' || num[start] == '-')) ++start;
        size_t end = num.size(); while (end > start && !(std::isdigit((unsigned char)num[end-1]) || num[end-1] == '.')) --end;
        if (start >= end) continue;
        std::string candidate = num.substr(start, end - start);
        try {
            size_t idx = 0;
            double val = std::stod(candidate, &idx);
            if (idx > 0) return val;
        } catch (...) { continue; }
    }
    return 0.0;
}

std::vector<std::shared_ptr<Transaction>> StatementParser::parseTransactions(const StatementData& statementData) {
    const auto& layouts = statementData.getOpenCvArtifacts();
    const auto& ocrTablesPerPage = statementData.getOcrArtifacts();

    std::vector<std::shared_ptr<Transaction>> out;

    for (size_t pi = 0; pi < ocrTablesPerPage.size(); ++pi) {
        const auto& pageTables = ocrTablesPerPage[pi];
        std::cout << "[StatementParser] Page " << (pi+1) << " tables=" << pageTables.size() << std::endl;
        for (size_t ti = 0; ti < pageTables.size(); ++ti) {
            const auto& tbl = pageTables[ti];
            std::cout << "[StatementParser]  Table " << (ti+1) << " bbox=(" << tbl.bbox.x << "," << tbl.bbox.y << ") rows=" << tbl.rows << " cols=" << tbl.cols << " cells=" << tbl.cells.size() << std::endl;
            if (tbl.rows <= 0 || tbl.cols <= 0) continue;

            auto headerRows = detectHeaderRows(tbl);
            int amountCol = detectAmountColumn(tbl);
            auto detailCols = detectDetailColumns(tbl, amountCol);

            std::cout << "[StatementParser]   Detected header rows:";
            for (int h : headerRows) std::cout << " " << h;
            std::cout << " amountCol=" << amountCol;
            std::cout << " detailCols:";
            for (int c : detailCols) std::cout << " " << c;
            std::cout << std::endl;

            // Build a map row -> vector<cell texts by col>
            std::map<int, std::map<int, std::string>> tableText;
            for (const auto& c : tbl.cells) {
                tableText[c.row][c.col] = c.text;
            }

            // For each data row, construct Transaction (simple heuristic)
            for (int r = 0; r < tbl.rows; ++r) {
                if (headerRows.count(r)) continue;
                // concatenate detail columns
                std::ostringstream details;
                bool first = true;
                for (int col : detailCols) {
                    auto itc = tableText[r].find(col);
                    if (itc == tableText[r].end()) continue;
                    std::string t = itc->second;
                    if (t.empty()) continue;
                    if (!first) details << " ";
                    details << t;
                    first = false;
                }
                std::string detailsStr = details.str();
                // amount
                std::string amountText;
                if (amountCol >= 0) {
                    auto it = tableText[r].find(amountCol);
                    if (it != tableText[r].end()) amountText = it->second;
                }
                // try extract date from row details
                std::string bookingDate = extractDate(detailsStr);

                // Debug print
                std::cout << "[StatementParser]   Row " << r << " bookingDate='" << bookingDate << "' details='" << detailsStr << "' amount='" << amountText << "'" << std::endl;

                // Create Transaction if there is some text or amount
                if (!detailsStr.empty() || !amountText.empty()) {
                    double amountVal = 0.0;
                    if (!amountText.empty()) amountVal = parseAmountLike(amountText);
                    std::map<std::string,std::string> meta;
                    if (!detailsStr.empty()) meta["description"] = detailsStr;
                    // construct shared_ptr explicitly to avoid make_shared overload issues on some toolchains
                    auto tx = std::shared_ptr<Transaction>(new Transaction(bookingDate, std::string(), amountVal, std::string(), std::move(meta)));
                    out.push_back(tx);
                }
            }
        }
    }

    return out;
}

} // namespace utils
