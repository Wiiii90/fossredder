#include "core/parser/DefaultTransactionParser.h"
#include "core/utils/Util.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <regex>
#include <string>
#include <vector>

using utils::collapseWhitespace;
using utils::splitWhitespace;
using utils::trim;

namespace core::parser {

namespace {

bool tightContinuation(const OcrLine& prev, const OcrLine& next) {
    const bool closeY = (next.minY - prev.maxY) <= 18;
    const bool leftAligned = std::abs(next.minX - prev.minX) <= 14;
    return closeY && leftAligned;
}

bool hasSingleTokenBeforeColon(const std::string& s, size_t colonPos) {
    if (colonPos == std::string::npos || colonPos == 0) return false;
    const auto left = trim(s.substr(0, colonPos));
    if (left.empty()) return false;
    return splitWhitespace(left).size() == 1;
}

bool looksLikeLabelStart(const std::string& line) {
    const auto pos = line.find(':');
    if (pos == std::string::npos || pos == 0 || pos > 40) return false;
    return hasSingleTokenBeforeColon(line, pos);
}

std::optional<std::string> parseValutaToken(const std::string& line) {
    static const std::regex reJoined(R"((\d{2}\.\d{2}))");
    static const std::regex reSplit(R"((\d{2})\.)");
    std::smatch m;

    if (std::regex_search(line, m, reJoined)) return m.str(1);

    // Handle split OCR tokens like "17." and "02" (line text seen as "17. 02")
    {
        std::smatch md;
        if (std::regex_search(line, md, reSplit)) {
            const auto day = md.str(1);
            const auto rest = md.suffix().str();
            static const std::regex reMonth(R"(\b(\d{2})\b)");
            std::smatch mm;
            if (std::regex_search(rest, mm, reMonth)) {
                return day + "." + mm.str(1);
            }
        }
    }

    return std::nullopt;
}

std::optional<double> parseAmountToken(const std::string& line) {
    static const std::regex re(R"((\d{1,3}(?:[\.,]\d{3})*,\d{2})(-)?)");
    std::smatch m;
    std::string s = line;
    std::optional<double> last;

    while (std::regex_search(s, m, re)) {
        std::string token = m.str(1);
        if (m.size() > 2 && m.str(2) == "-") token.push_back('-');

        bool negative = false;
        if (!token.empty() && token.back() == '-') {
            negative = true;
            token.pop_back();
        }

        token.erase(std::remove(token.begin(), token.end(), '.'), token.end());
        std::replace(token.begin(), token.end(), ',', '.');

        try {
            double v = std::stod(token);
            if (negative) v = -v;
            last = v;
        } catch (...) {
        }

        s = m.suffix().str();
    }

    return last;
}

int textRightEdge(const OcrLine& l) {
    if (!l.wordSpans.empty()) return utils::rightEdgeFromWordSpans(l.wordSpans);
    return l.maxX;
}

} // namespace

DefaultTransactionParser DefaultTransactionParser::parseTransaction(const TransactionBlock& block) {
    DefaultTransactionParser tx;
    tx.bookingDate = block.bookingDateGroup;

    if (!block.main.valuta.empty()) {
        if (auto v = parseValutaToken(block.main.valuta.line.text)) tx.valuta = *v;
    }

    if (!block.main.credit.empty()) {
        if (auto v = parseAmountToken(block.main.credit.line.text)) tx.amount = *v;
    }
    if (tx.amount == 0.0 && !block.main.debit.empty()) {
        if (auto v = parseAmountToken(block.main.debit.line.text)) tx.amount = -std::abs(*v);
    }

    std::vector<OcrLine> lines;
    if (!block.main.left.empty()) lines.push_back(block.main.left.line);
    for (const auto& l : block.detailLines) {
        if (!trim(l.text).empty()) lines.push_back(l);
    }
    if (lines.empty()) return tx;

    int rightEdge = 0;
    for (const auto& l : lines) rightEdge = std::max(rightEdge, textRightEdge(l));

    std::vector<std::string> nameLines;
    nameLines.push_back(collapseWhitespace(lines.front().text));

    for (size_t i = 1; i < lines.size(); ++i) {
        const auto& prev = lines[i - 1];
        const auto& cur = lines[i];

        const int prevRight = textRightEdge(prev);
        const int gap = std::max(0, rightEdge - prevRight);
        const int w = std::max(1, rightEdge - lines.front().minX);

        const int allowAbs = std::max(24, w / 40);
        const int allowRel = w / 25;
        const int allow = std::min(std::max(1, allowAbs), std::max(1, allowRel));

        if (gap > allow) break;
        if (!tightContinuation(prev, cur)) break;

        nameLines.push_back(collapseWhitespace(cur.text));
    }

    std::string nm;
    for (const auto& l : nameLines) nm += l;
    tx.name = collapseWhitespace(std::move(nm));
    if (tx.name.size() > 120) tx.name.resize(120);
    tx.actorProposal = tx.name;

    std::string meta;
    bool lastWasLabel = false;
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string t = collapseWhitespace(lines[i].text);
        t = trim(std::move(t));
        if (t.empty()) continue;
        const bool isLabel = looksLikeLabelStart(t);

        if (i == nameLines.size()) {
            meta.push_back('\n');
        } else {
            if (!meta.empty()) {
                if (isLabel || lastWasLabel) meta.push_back('\n');
            }
        }

        meta += t;
        lastWasLabel = isLabel;
    }
    tx.metadata = trim(std::move(meta));

    return tx;
}

} // namespace core::parser
