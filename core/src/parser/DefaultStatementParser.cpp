#include "core/pch.h"
#include "core/parser/DefaultStatementParser.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <regex>
#include <sstream>

namespace {

static std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static std::string lowerAscii(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

static std::optional<std::string> findBookingDate(const std::string& line) {
    auto l = lowerAscii(line);
    if (l.find("buchungsdatum") == std::string::npos) return std::nullopt;

    std::regex re(R"((\d{2}\.\d{2}\.\d{4}|\d{2}\.\d{2}\.\d{2}|\d{2}\.\d{2}))");
    std::smatch m;
    if (std::regex_search(line, m, re)) return m.str(1);
    return std::nullopt;
}

static std::optional<std::string> findValuta(const std::string& line) {
    std::regex re(R"((\d{2}\.\d{2}))");
    std::smatch m;
    if (std::regex_search(line, m, re)) return m.str(1);
    return std::nullopt;
}

static std::optional<double> parseAmountToken(const std::string& token) {
    std::string t;
    t.reserve(token.size());
    for (char ch : token) {
        if (std::isdigit(static_cast<unsigned char>(ch)) || ch == ',' || ch == '.' || ch == '-') t.push_back(ch);
    }
    if (t.empty()) return std::nullopt;

    bool negative = false;
    if (!t.empty() && t.back() == '-') { negative = true; t.pop_back(); }

    t.erase(std::remove(t.begin(), t.end(), '.'), t.end());
    std::replace(t.begin(), t.end(), ',', '.');

    try {
        double v = std::stod(t);
        if (negative) v = -v;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

static std::optional<double> findAmount(const std::string& line) {
    // rightmost amount token
    std::regex re(R"((\d{1,3}(?:[\.,]\d{3})*,\d{2})(-)?)");
    std::smatch m;
    std::string s = line;
    std::optional<double> last;
    while (std::regex_search(s, m, re)) {
        std::string token = m.str(1);
        if (m.size() > 2 && m.str(2) == "-") token += "-";
        last = parseAmountToken(token);
        s = m.suffix().str();
    }
    return last;
}

struct Line {
    int cy = 0;
    std::string text;
};

static std::vector<Line> buildLines(const std::vector<api::tesseract::Word>& words) {
    std::vector<Line> out;

    struct WRef { const api::tesseract::Word* w; int cy; };
    std::vector<WRef> ws;
    ws.reserve(words.size());
    for (const auto& w : words) ws.push_back({ &w, w.bbox.y + w.bbox.height / 2 });

    std::sort(ws.begin(), ws.end(), [](const WRef& a, const WRef& b) {
        if (a.cy != b.cy) return a.cy < b.cy;
        return a.w->bbox.x < b.w->bbox.x;
    });

    int avgH = 0;
    for (const auto& ww : ws) avgH += ww.w->bbox.height;
    avgH = ws.empty() ? 10 : std::max(1, avgH / static_cast<int>(ws.size()));
    int tol = std::max(6, avgH / 2);

    for (const auto& ww : ws) {
        if (out.empty() || std::abs(ww.cy - out.back().cy) > tol) {
            Line l;
            l.cy = ww.cy;
            l.text = ww.w->text;
            out.push_back(std::move(l));
        } else {
            auto& l = out.back();
            l.cy = (l.cy + ww.cy) / 2;
            if (!l.text.empty()) l.text.push_back(' ');
            l.text += ww.w->text;
        }
    }
    return out;
}

static bool isFooterLine(const std::string& line) {
    auto l = lowerAscii(line);
    return l.find("folgeseite") != std::string::npos;
}

} // namespace

DefaultStatementParser::ParseResult DefaultStatementParser::parse(const api::opencv::Table& /*table*/, const api::tesseract::ExtractResult& ocr, std::string initialBookingDate) {
    ParseResult out;

    auto lines = buildLines(ocr.words);
    out.debugLines.push_back("META\tlines\t" + std::to_string(lines.size()));

    std::string currentBookingDate = std::move(initialBookingDate);
    bool inTransactions = !currentBookingDate.empty();
    if (inTransactions) out.debugLines.push_back("CTX\tinitialBookingDate\t" + currentBookingDate);

    for (const auto& l : lines) {
        auto txt = trim(l.text);
        if (txt.empty() || isFooterLine(txt)) continue;

        out.debugLines.push_back("LINE\t" + std::to_string(l.cy) + "\t" + txt);

        if (auto bd = findBookingDate(txt)) {
            currentBookingDate = *bd;
            inTransactions = true;
            out.debugLines.push_back("BOOKINGDATE\t" + currentBookingDate + "\tlineY\t" + std::to_string(l.cy));
            continue;
        }

        if (!inTransactions) continue;

        auto amount = findAmount(txt);
        auto valuta = findValuta(txt);

        if (amount && valuta) {
            Transaction tx;
            tx.bookingDate = currentBookingDate;
            tx.valuta = *valuta;
            tx.amount = *amount;
            tx.name = txt;
            out.transactions.push_back(std::move(tx));
            out.debugLines.push_back("TX_START\tlineY\t" + std::to_string(l.cy) + "\tvaluta\t" + *valuta + "\tamount\t" + std::to_string(*amount));
        } else if (!out.transactions.empty()) {
            out.transactions.back().name += "\n" + txt;
            out.debugLines.push_back("TX_CONT\tlineY\t" + std::to_string(l.cy));
        }
    }

    out.lastBookingDate = currentBookingDate;
    out.debugLines.push_back("CTX\tlastBookingDate\t" + out.lastBookingDate);
    out.debugLines.push_back("META\ttxCount\t" + std::to_string(out.transactions.size()));
    return out;
}
