#include "core/parser/DefaultTransactionParser.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/utils/Util.h"
#include "core/parser/ParserHelpers.h"

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

std::optional<double> parseAmountInternal(const std::string& line) {
    // normalize some unicode hyphens/minus signs and NBSP
    std::string s = line;
    try {
        // replace common unicode minus/dash variants with ASCII '-'
        auto replaceAll = [&](const std::string& from, const std::string& to) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos) {
                s.replace(pos, from.size(), to);
                pos += to.size();
            }
        };
        replaceAll("\xE2\x80\x93", "-"); // en dash
        replaceAll("\xE2\x80\x94", "-"); // em dash
        replaceAll("\xE2\x88\x92", "-"); // minus sign
        // NBSP (0xC2 0xA0)
        replaceAll("\xC2\xA0", " ");
        replaceAll("\xE2\x80\xAF", " "); // narrow no-break space U+202F
        replaceAll("\xE2\x80\x89", " "); // thin space U+2009
        // normalize some common OCR dot variants to ASCII dot
        replaceAll("\xE2\x80\xA2", "."); // bullet -> dot
        replaceAll("\xE2\x80\xB7", "."); // interpunct
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::normalizeChars", std::current_exception()); }

    // Additional sanitization: remove any characters that are not digits, dot, comma, minus, parentheses or whitespace
    std::string clean;
    clean.reserve(s.size());
    for (unsigned char c : s) {
        if ((c >= '0' && c <= '9') || c == '.' || c == ',' || c == '-' || c == '(' || c == ')' || std::isspace(c)) clean.push_back(static_cast<char>(c));
        else {
            // replace other characters with space to avoid token concatenation
            clean.push_back(' ');
        }
    }
    s = std::move(clean);

    // Repair common OCR splits: e.g. "10,0 0-" -> "10,00-"
    try {
        static const std::regex splitDec(R"((\d{1,3}[\.,]\d)\s+(\d{1,2}-?))");
        std::smatch m;
        std::string tmp = s;
        // apply repeatedly to catch multi-splits
        while (std::regex_search(tmp, m, splitDec)) {
            const std::string repl = m.str(1) + m.str(2);
            tmp = m.prefix().str() + repl + m.suffix().str();
        }
        s = std::move(tmp);
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::repairSplit", std::current_exception()); }

    std::optional<double> last;

    // Primary regex: numbers with comma as decimal, optional thousand separators
    static const std::regex re(R"((\(?-?(?:\d{1,3}(?:[\.,]\d{3})*|\d+),\d{2}\)?))");
    try {
        // collect all candidate matches and pick best by magnitude to avoid small substring matches
        std::smatch m;
        std::optional<double> best;
        double bestAbs = 0.0;
        for (std::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it) {
            try {
                std::string token = (*it).str(1);
                if (core::parser::helpers::containsShortDate(token)) continue;

                bool negative = false;
                if (!token.empty() && token.front() == '(' && token.back() == ')') { negative = true; token = token.substr(1, token.size() - 2); }
                if (!token.empty() && token.front() == '-') { negative = true; token = token.substr(1); }
                if (!token.empty() && token.back() == '-') { negative = true; token.pop_back(); }

                // remove thousand dots and normalize comma to dot
                std::string normalized = token;
                normalized.erase(std::remove(normalized.begin(), normalized.end(), '.'), normalized.end());
                std::replace(normalized.begin(), normalized.end(), ',', '.');

                try {
                    double v = std::stod(normalized);
                    if (negative) v = -v;
                    double av = std::abs(v);
                    if (!best || av > bestAbs) { best = v; bestAbs = av; }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::stodPrimary", std::current_exception()); }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::tokenPrimary", std::current_exception()); }
        }
        if (best) return best;
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::primary", std::current_exception()); }

    // Fallback: find simple forms like 59268,40 or -59268,40
    if (!last) {
        static const std::regex re2(R"((\(?-?\d+[\.,]\d{2}\)?))");
        try {
            std::optional<double> best;
            double bestAbs = 0.0;
            for (std::sregex_iterator it(s.begin(), s.end(), re2), end; it != end; ++it) {
                try {
                    std::string token = (*it).str(1);
                    if (core::parser::helpers::containsShortDate(token)) continue;
                    bool negative = false;
                    if (!token.empty() && token.front() == '(' && token.back() == ')') { negative = true; token = token.substr(1, token.size() - 2); }
                    if (!token.empty() && token.front() == '-') { negative = true; token = token.substr(1); }
                    token.erase(std::remove(token.begin(), token.end(), '.'), token.end());
                    std::replace(token.begin(), token.end(), ',', '.');
                    try {
                        double v = std::stod(token);
                        if (negative) v = -v;
                        double av = std::abs(v);
                        if (!best || av > bestAbs) { best = v; bestAbs = av; }
                    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::stodFallback", std::current_exception()); }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::tokenFallback", std::current_exception()); }
            }
            if (best) return best;
        } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseAmountInternal::fallback", std::current_exception()); }
    }

    return std::nullopt;
}

} // anonymous namespace

// exported wrapper in core::parser namespace
std::optional<double> parseAmountString(const std::string& line) {
    return parseAmountInternal(line);
}

// helper to compute right edge from OcrLine
int textRightEdge(const OcrLine& l) {
    if (!l.wordSpans.empty()) return utils::rightEdgeFromWordSpans(l.wordSpans);
    return l.maxX;
}

DefaultTransactionParser DefaultTransactionParser::parseTransaction(const TransactionBlock& block, std::vector<std::string>* debugOut /*=nullptr*/) {
    DefaultTransactionParser tx;
    tx.bookingDate = block.bookingDateGroup;

    if (!block.main.valuta.empty()) {
        if (auto v = parseValutaToken(block.main.valuta.line.text)) tx.valuta = *v;
    }

    bool sourceDebit = false;
    bool sourceCredit = false;

    // Determine amounts from credit/debit tokens with robust sign heuristics
    auto tokenIndicatesNegative = [&](const std::string &txt)->bool{
        std::string s = txt;
        try { s = utils::trim(s); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::trim", std::current_exception()); }
        if (s.empty()) return false;
        if (s.front() == '(' && s.back() == ')') return true;
        if (s.front() == '-') return true;
        if (s.back() == '-') return true;
        return false;
    };

    auto centerXOf = [&](const OcrLine& l)->int{
        if (!l.wordSpans.empty()) return (l.wordSpans.front().first + l.wordSpans.back().second) / 2;
        return (l.minX + l.maxX) / 2;
    };

    std::optional<double> creditVal;
    std::optional<double> debitVal;
    bool creditExplicitNeg = false;
    bool debitExplicitNeg = false;

    if (!block.main.credit.empty()) {
        creditExplicitNeg = tokenIndicatesNegative(block.main.credit.line.text);
        // Prefer robust helper that works on OcrLine tokens
        if (auto v = core::parser::helpers::findAndParseAmountInLine(block.main.credit.line, -1, debugOut)) {
            creditVal = v;
            if (debugOut) debugOut->push_back(std::string("initial.credit.helperUsed\t") + block.main.credit.line.text + std::string(" -> ") + std::to_string(*creditVal));
        } else {
            creditVal = parseAmountString(block.main.credit.line.text);
            if (debugOut) debugOut->push_back(std::string("initial.credit.parse\t") + block.main.credit.line.text + std::string(" -> ") + (creditVal ? std::to_string(*creditVal) : std::string("(none)")));
        }
    }
    if (!block.main.debit.empty()) {
        debitExplicitNeg = tokenIndicatesNegative(block.main.debit.line.text);
        if (auto v = core::parser::helpers::findAndParseAmountInLine(block.main.debit.line, -1, debugOut)) {
            debitVal = v;
            if (debugOut) debugOut->push_back(std::string("initial.debit.helperUsed\t") + block.main.debit.line.text + std::string(" -> ") + std::to_string(*debitVal));
        } else {
            debitVal = parseAmountString(block.main.debit.line.text);
            if (debugOut) debugOut->push_back(std::string("initial.debit.parse\t") + block.main.debit.line.text + std::string(" -> ") + (debitVal ? std::to_string(*debitVal) : std::string("(none)")));
        }
    }

    // Fallback: if neither side parsed or parsed values look suspiciously small, try a more robust scan
    try {
        const double suspiciousThreshold = 1.0; // amounts smaller than this considered suspect for fallback
        bool needFallback = (!creditVal && !debitVal);
        if (!needFallback) {
            if (creditVal && std::abs(*creditVal) < suspiciousThreshold) needFallback = true;
            if (debitVal && std::abs(*debitVal) < suspiciousThreshold) needFallback = true;
        }
        if (needFallback) {
            if (debugOut) debugOut->push_back(std::string("fallback.scan.start\tcreditVal=") + (creditVal?"1":"0") + std::string(" debitVal=") + (debitVal?"1":"0"));
            // scan main line cells and detail lines for amount-like tokens
            if (!block.main.credit.empty()) {
                if (auto v = core::parser::helpers::findAndParseAmountInLine(block.main.credit.line, -1, debugOut)) { creditVal = v; if (debugOut) debugOut->push_back(std::string("fallback.credit.helper\t") + block.main.credit.line.text + std::string(" -> ") + std::to_string(*v)); }
            }
            if (!creditVal && !block.main.debit.empty()) {
                if (auto v = core::parser::helpers::findAndParseAmountInLine(block.main.debit.line, -1, debugOut)) { debitVal = v; if (debugOut) debugOut->push_back(std::string("fallback.debit.helper\t") + block.main.debit.line.text + std::string(" -> ") + std::to_string(*v)); }
            }
            if (!creditVal && !debitVal) {
                // scan left and detail lines
                if (!block.main.left.empty()) {
                    if (auto v = core::parser::helpers::findAndParseAmountInLine(block.main.left.line, -1, debugOut)) {
                        // ambiguous which column; assume credit unless signage indicates debit
                        creditVal = v;
                        if (debugOut) debugOut->push_back(std::string("fallback.left.helper\t") + block.main.left.line.text + std::string(" -> ") + std::to_string(*v));
                    }
                }
                for (const auto& dl : block.detailLines) {
                    if (creditVal || debitVal) break;
                    if (auto v = core::parser::helpers::findAndParseAmountInLine(dl, -1, debugOut)) { creditVal = v; if (debugOut) debugOut->push_back(std::string("fallback.detail.helper\t") + dl.text + std::string(" -> ") + std::to_string(*v)); break; }
                }
            }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::fallbackScan", std::current_exception()); }

    // If both parsed, resolve using explicit markers or magnitude
    if (creditVal && debitVal) {
        if (creditExplicitNeg && !debitExplicitNeg) {
            tx.amount = *creditVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.creditNeg->") + std::to_string(tx.amount));
        } else if (debitExplicitNeg && !creditExplicitNeg) {
            tx.amount = *debitVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.debitNeg->") + std::to_string(tx.amount));
        } else {
            // prefer larger absolute amount as likely the real amount
            if (std::abs(*debitVal) >= std::abs(*creditVal)) tx.amount = *debitVal; else tx.amount = *creditVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.larger->") + std::to_string(tx.amount));
        }
    } else if (creditVal) {
        // only credit parsed -> use as-is
        tx.amount = *creditVal;
        if (debugOut) debugOut->push_back(std::string("resolve.credit->") + std::to_string(tx.amount));
    } else if (debitVal) {
        // only debit parsed -> decide sign by relative horizontal position if possible
        bool treatAsCredit = false;
        if (!block.main.credit.empty()) {
            try {
                int cxDebit = centerXOf(block.main.debit.line);
                int cxCredit = centerXOf(block.main.credit.line);
                if (cxDebit > cxCredit) treatAsCredit = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::debitVsCreditCenter", std::current_exception()); }
        }
        if (tokenIndicatesNegative(block.main.debit.line.text)) {
            tx.amount = *debitVal; // keep explicit sign
            if (debugOut) debugOut->push_back(std::string("resolve.debitExpNeg->") + std::to_string(tx.amount));
        } else if (treatAsCredit) {
            tx.amount = *debitVal; // treat as positive credit
            if (debugOut) debugOut->push_back(std::string("resolve.debit.treatAsCredit->") + std::to_string(tx.amount));
        } else {
            tx.amount = -std::abs(*debitVal); // default: debit -> negative
            if (debugOut) debugOut->push_back(std::string("resolve.debit.defaultNegative->") + std::to_string(tx.amount));
        }
    }

    // If still zero, try to combine split tokens (debit+credit or credit+debit)
    if (tx.amount == 0.0 && !block.main.debit.empty() && !block.main.credit.empty()) {
        auto tryCombineParse = [&](const std::string& a, const std::string& b)->std::optional<double> {
            try {
                if (a.empty() || b.empty()) return std::nullopt;
                if (auto p = parseAmountString(a + b)) return p;
                if (auto p2 = parseAmountString(a + " " + b)) return p2;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::tryCombineParse", std::current_exception()); }
            return std::nullopt;
        };
        if (auto p = tryCombineParse(block.main.debit.line.text, block.main.credit.line.text)) {
            // debit+credit combined: assume belongs to debit column unless credit clearly right of debit
            bool creditRightOfDebit = false;
            try { creditRightOfDebit = centerXOf(block.main.credit.line) > centerXOf(block.main.debit.line); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::combineCenter", std::current_exception()); }
            if (creditRightOfDebit) tx.amount = *p; else tx.amount = -std::abs(*p);
            if (debugOut) debugOut->push_back(std::string("combine.debit+credit->") + std::to_string(tx.amount));
        } else if (auto p = tryCombineParse(block.main.credit.line.text, block.main.debit.line.text)) {
            tx.amount = *p;
            if (debugOut) debugOut->push_back(std::string("combine.credit+debit->") + std::to_string(tx.amount));
        }
    }

    // Enforce explicit negative markers: if any token shows '-' or parentheses, make amount negative
    try {
        auto hasNegMarker = [&](const std::string &t)->bool{
            if (t.empty()) return false;
            for (char c : t) if (c == '-' || c == '(' || c == ')') return true;
            return false;
        };
        if (tx.amount > 0.0 && (hasNegMarker(block.main.debit.line.text) || hasNegMarker(block.main.credit.line.text))) {
            tx.amount = -std::abs(tx.amount);
            if (debugOut) debugOut->push_back(std::string("enforce.negMarker->") + std::to_string(tx.amount));
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::negMarker", std::current_exception()); }

    // Enforce column semantics: debit column -> negative, credit column -> positive
    try {
        if (sourceDebit && sourceCredit) {
            // ambiguous: do not change sign
        } else if (sourceDebit) {
            if (tx.amount > 0.0) tx.amount = -std::abs(tx.amount);
        } else if (sourceCredit) {
            if (tx.amount < 0.0) tx.amount = std::abs(tx.amount);
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultTransactionParser::parseTransaction::columnSemantics", std::current_exception()); }

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
