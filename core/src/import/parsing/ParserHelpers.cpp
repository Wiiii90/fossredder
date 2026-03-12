#include "ParserHelpers.h"
#include "core/utils/Util.h"
#include "DefaultTransactionParser.h"
#include "DefaultStatementParser.h"
#include "ParserDateUtils.h"
#include "core/import/ParserHeuristics.h"
#include "core/errors/ErrorReporterRegistry.h"
#include <sstream>

namespace core::parser::helpers {
// define global config instance
ParserConfig parserConfig;
// Central constants for amount and date detection
namespace {
    static const std::regex g_amountRegex(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?$)");
    static const std::regex g_amountFallbackRegex(R"(^\(?-?\d+[\.,]\d{2}\)?$)");
    // require word boundaries so we don't match short-date inside larger numeric tokens (e.g. 17.072,66)
    bool tokenLooksLikeAmount(const std::string& token) noexcept {
        try {
            return std::regex_match(token, g_amountRegex) || std::regex_match(token, g_amountFallbackRegex);
        } catch (...) { return false; }
    }
}

std::vector<size_t> findAmountTokenIndices(const core::parser::OcrLine& line, int valutaX, int bandPx) noexcept {
    std::vector<size_t> out;
    try {
        const auto toks = utils::splitWhitespace(line.text);
        for (size_t i = 0; i < toks.size(); ++i) {
            try {
                if (!tokenLooksLikeAmount(toks[i])) continue;
            } catch (...) { continue; }
            if (valutaX >= 0 && bandPx > 0) {
                if (i >= line.wordSpans.size()) continue;
                const auto& sp = line.wordSpans[i];
                const int cx = (sp.first + sp.second) / 2;
                if (std::abs(cx - valutaX) > bandPx) continue;
            }
            out.push_back(i);
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAmountTokenIndices", std::current_exception()); }
    return out;
}

std::string normalizeAlnumLower(const std::string& s) noexcept {
    std::string o;
    o.reserve(s.size());
    for (unsigned char c : utils::lowerAscii(s)) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) o.push_back(static_cast<char>(c));
    }
    return o;
}

std::optional<int> findTokenCenterX(const core::parser::OcrLine& line, const std::string& tokenLower) noexcept {
    const auto toks = utils::splitWhitespace(line.text);
    if (toks.size() != line.wordSpans.size()) return std::nullopt;

    const auto want = normalizeAlnumLower(tokenLower);
    for (size_t i = 0; i < toks.size(); ++i) {
        if (normalizeAlnumLower(toks[i]) != want) continue;
        const auto& sp = line.wordSpans[i];
        return (sp.first + sp.second) / 2;
    }

    for (size_t i = 0; i < toks.size(); ++i) {
        const auto n = normalizeAlnumLower(toks[i]);
        if (n.find(want) == std::string::npos) continue;
        const auto& sp = line.wordSpans[i];
        return (sp.first + sp.second) / 2;
    }

    return std::nullopt;
}

std::optional<int> findPhraseCenterX(const core::parser::OcrLine& line, std::initializer_list<const char*> phraseLower) noexcept {
    const auto toks = utils::splitWhitespace(line.text);
    if (toks.size() != line.wordSpans.size()) return std::nullopt;

    std::string want;
    for (auto w : phraseLower) want += normalizeAlnumLower(std::string(w));
    if (want.empty() || toks.empty()) return std::nullopt;

    for (size_t i = 0; i < toks.size(); ++i) {
        std::string acc;
        int left = line.wordSpans[i].first;
        int right = line.wordSpans[i].second;

        for (size_t j = i; j < toks.size() && (j - i) < static_cast<size_t>(parserConfig.maxPhraseTokens); ++j) {
            acc += normalizeAlnumLower(toks[j]);
            right = line.wordSpans[j].second;
            if (acc.find(want) != std::string::npos) {
                return (left + right) / 2;
            }
        }
    }

    return std::nullopt;
}

bool hasTokenNearX(const core::parser::OcrLine& line, int x, int bandPx) noexcept {
    if (x < 0) return false;
    const auto toks = utils::splitWhitespace(line.text);
    if (toks.size() != line.wordSpans.size() || toks.empty()) return false;

    for (size_t i = 0; i < toks.size(); ++i) {
        const auto& sp = line.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (std::abs(cx - x) <= bandPx) return true;
    }
    return false;
}

bool hasAmountLikeTokenInLine(const core::parser::OcrLine& line, int valutaX) noexcept {
    try {
        if (valutaX >= 0) {
            return !findAmountTokenIndices(line, valutaX, parserConfig.amountNearValutaBandPx).empty();
        }
        return !findAmountTokenIndices(line, -1, 0).empty();
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::hasAmountLikeTokenInLine", std::current_exception()); }
    return false;
}

bool hasLeftDescriptiveText(const core::parser::OcrLine& line, int valutaX) noexcept {
    try {
        const auto toks = utils::splitWhitespace(line.text);
        if (toks.empty()) return false;
        for (size_t i = 0; i < toks.size() && i < line.wordSpans.size(); ++i) {
            const auto& sp = line.wordSpans[i];
            const int cx = (sp.first + sp.second) / 2;
            if (cx < valutaX - parserConfig.leftDescriptiveOffsetPx) {
                for (unsigned char c : toks[i]) if (std::isalpha(c)) return true;
            }
        }
        // fallback: first token contains letters
        for (unsigned char c : toks[0]) if (std::isalpha(c)) return true;
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::hasLeftDescriptiveText", std::current_exception()); }
    return false;
}

bool hasAmountNearValuta(const core::parser::OcrLine& line, int valutaX, int bandPx) noexcept {
    if (valutaX < 0) return false;
    try {
        auto idxs = findAmountTokenIndices(line, valutaX, bandPx);
        return !idxs.empty();
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::hasAmountNearValuta", std::current_exception()); }
    return false;
}

bool isLooseTransactionLine(const core::parser::OcrLine& line, int valutaX) noexcept {
    if (valutaX < 0) return false;
    try {
        if (!hasShortDateToken(line.text)) return false;
        std::smatch md;
        if (!std::regex_search(line.text, md, std::regex(R"((\d{2}\.\s*\d{2}))"))) return false;
        const size_t datePos = static_cast<size_t>(md.position(0));
        const auto toks = utils::splitWhitespace(line.text);
        bool foundAmountAfterDate = false;
        // use consolidated finder to check for any amount tokens (no valuta constraint here)
        auto amtIdxs = findAmountTokenIndices(line, -1, 0);
        for (auto i : amtIdxs) {
            if (i >= toks.size()) continue;
            const auto pos = line.text.find(toks[i]);
            if (pos != std::string::npos && pos > datePos) { foundAmountAfterDate = true; break; }
        }
        if (!foundAmountAfterDate) return false;
        const auto left = line.text.substr(0, datePos);
        for (unsigned char c : left) { if (std::isalpha(c)) return true; }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::isLooseTransactionLine", std::current_exception()); }
    return false;
}

// Improved implementation: try multiple token combination strategies to handle split/truncated amounts
std::optional<double> findAndParseAmountInLine(const core::parser::OcrLine& line, int valutaX, std::vector<std::string>* debugOut) noexcept {
    try {
        int band = (valutaX >= 0) ? parserConfig.amountNearValutaBandPx : 0;
        auto idxs = findAmountTokenIndices(line, valutaX, band);
        if (idxs.empty()) {
            // fallback: consider any amount-like token
            idxs = findAmountTokenIndices(line, -1, 0);
        }
        const auto toks = utils::splitWhitespace(line.text);
        auto tryParse = [&](const std::string& s)->std::optional<double> {
            try {
                if (s.empty()) return std::nullopt;
                if (debugOut) debugOut->push_back(std::string("candidate.try\t") + s);

                // skip short date tokens like '25.04' which could be mistaken for amounts
                try {
                    if (core::parser::helpers::containsShortDate(s)) {
                        if (debugOut) debugOut->push_back(std::string("candidate.skip.shortDate\t") + s);
                        return std::nullopt;
                    }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAndParseAmountInLine::containsShortDate", std::current_exception()); }

                // First try: explicit normalization targeted at common European format
                try {
                    std::string norm = s;
                    // trim
                    norm = utils::trim(norm);
                    bool negative = false;
                    if (!norm.empty() && norm.front() == '(' && norm.back() == ')') { negative = true; norm = norm.substr(1, norm.size() - 2); }
                    if (!norm.empty() && norm.front() == '-') { negative = true; norm = norm.substr(1); }
                    if (!norm.empty() && norm.back() == '-') { negative = true; norm.pop_back(); }

                    // remove spaces and thousand separators (dots and thin spaces)
                    std::string tmp;
                    tmp.reserve(norm.size());
                    for (unsigned char c : norm) {
                        if (c == '.' || c == '\u202F' || c == '\u2009' || c == ' ') continue; // drop thousand separators
                        tmp.push_back(static_cast<char>(c));
                    }
                    // replace comma with dot for decimal
                    std::replace(tmp.begin(), tmp.end(), ',', '.');

                    // guard: don't consider pure integer-looking results that came from dates (e.g. 2504)
                    try {
                        if (!tmp.empty()) {
                            // if original looked like a date (two digits dot two digits) we already skipped; now try stod
                            double v = std::stod(tmp);
                            if (negative) v = -v;
                            if (debugOut) debugOut->push_back(std::string("candidate.ok.norm\t") + tmp + std::string(" -> ") + std::to_string(v));
                            return v;
                        }
                    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAndParseAmountInLine::stod", std::current_exception()); }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAndParseAmountInLine::normalize", std::current_exception()); }

                // fallback: try library parser on original
                if (auto v = ::core::parser::parseAmountString(s)) { if (debugOut) debugOut->push_back(std::string("candidate.ok\t") + s + std::string(" -> ") + std::to_string(*v)); return v; }

                // also try removing spaces and feeding to library
                std::string nosp = s; nosp.erase(std::remove(nosp.begin(), nosp.end(), ' '), nosp.end());
                if (!nosp.empty()) { if (debugOut) debugOut->push_back(std::string("candidate.try.nosp\t") + nosp); if (auto v2 = ::core::parser::parseAmountString(nosp)) { if (debugOut) debugOut->push_back(std::string("candidate.ok.nosp\t") + nosp + std::string(" -> ") + std::to_string(*v2)); return v2; } }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAndParseAmountInLine::tryParse", std::current_exception()); }
            return std::nullopt;
        };

        if (debugOut) debugOut->push_back(std::string("helper.findAndParseAmountInLine\tline=") + line.text + std::string(" idxs=") + std::to_string(idxs.size()));

        for (auto i : idxs) {
            if (i >= toks.size()) continue;
            // try single token
            if (auto r = tryParse(toks[i])) return r;
            // try previous + current
            if (i > 0) {
                if (auto r = tryParse(toks[i-1] + toks[i])) return r;
                if (auto r2 = tryParse(toks[i-1] + " " + toks[i])) return r2;
            }
            // try current + next (up to 2 next tokens) to handle splits like "10,0" "0-"
            for (int len = 1; len <= 2; ++len) {
                std::string joined;
                for (int k = 0; k <= len; ++k) {
                    int idx = static_cast<int>(i) + k;
                    if (idx >= static_cast<int>(toks.size())) break;
                    if (!joined.empty()) joined.push_back(' ');
                    joined += toks[idx];
                }
                if (!joined.empty()) {
                    if (auto r = tryParse(joined)) return r;
                }
            }
            // try joining up to two tokens before and after (three-token window)
            if (i + 2 < toks.size()) {
                std::string three = toks[i] + toks[i+1] + toks[i+2];
                if (auto r = tryParse(three)) return r;
            }
        }
        // As a last resort, try to run parseAmountString on whole line
        if (debugOut) debugOut->push_back(std::string("candidate.try.whole\t") + line.text);
        if (auto v = ::core::parser::parseAmountString(line.text)) { if (debugOut) debugOut->push_back(std::string("candidate.ok.whole\t") + line.text + std::string(" -> ") + std::to_string(*v)); return v; }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findAndParseAmountInLine", std::current_exception()); }
    return std::nullopt;
}

// Build OcrLine vector from tesseract words (reuse logic from DefaultStatementParser)
std::vector<core::parser::OcrLine> buildOcrLinesFromWords(const std::vector<api::tesseract::Word>& words) {
    // reuse the same logic as DefaultStatementParser::buildOcrLines but produce OcrLine
    struct W { const api::tesseract::Word* w; int cy; int top; int bottom; int left; };
    std::vector<W> ws; ws.reserve(words.size());
    for (const auto& w : words) {
        bool anyNonSpace = false;
        for (unsigned char c : w.text) { if (!std::isspace(c)) { anyNonSpace = true; break; } }
        if (!anyNonSpace) continue;
        const int cy = w.bbox.y + w.bbox.height / 2;
        ws.push_back(W{ &w, cy, w.bbox.y, w.bbox.y + w.bbox.height, w.bbox.x });
    }
    std::sort(ws.begin(), ws.end(), [](const W& a, const W& b) { if (a.top != b.top) return a.top < b.top; if (a.cy != b.cy) return a.cy < b.cy; return a.left < b.left; });
    int avgH = 0; for (const auto& ww : ws) avgH += (ww.bottom - ww.top);
    avgH = ws.empty() ? 10 : std::max(1, avgH / static_cast<int>(ws.size()));
    const int minOverlap = std::max(2, avgH / 3);
    struct LineAcc { int cy = 0; int minX = std::numeric_limits<int>::max(); int maxX = std::numeric_limits<int>::min(); int minY = std::numeric_limits<int>::max(); int maxY = std::numeric_limits<int>::min(); std::vector<const api::tesseract::Word*> words; };
    std::vector<LineAcc> acc; acc.reserve(std::max<size_t>(8, words.size() / 8));
    for (const auto& ww : ws) {
        const auto& w = *ww.w;
        const int wMinX = w.bbox.x; const int wMaxX = w.bbox.x + w.bbox.width; const int wMinY = w.bbox.y; const int wMaxY = w.bbox.y + w.bbox.height;
        int bestIdx = -1; int bestOverlap = -1;
        for (int i = static_cast<int>(acc.size()) - 1; i >= 0; --i) {
            const auto& l = acc[static_cast<size_t>(i)];
            const int top = std::max(l.minY, wMinY);
            const int bot = std::min(l.maxY, wMaxY);
            const int ov = bot - top;
            if (ov >= minOverlap && ov > bestOverlap) { bestOverlap = ov; bestIdx = i; }
            if (wMinY - l.maxY > avgH) break;
        }
        if (bestIdx < 0) { LineAcc nl; nl.cy = ww.cy; nl.minX = wMinX; nl.maxX = wMaxX; nl.minY = wMinY; nl.maxY = wMaxY; nl.words.push_back(&w); acc.push_back(std::move(nl)); continue; }
        auto& l = acc[static_cast<size_t>(bestIdx)]; l.cy = (l.cy + ww.cy) / 2; l.minX = std::min(l.minX, wMinX); l.maxX = std::max(l.maxX, wMaxX); l.minY = std::min(l.minY, wMinY); l.maxY = std::max(l.maxY, wMaxY); l.words.push_back(&w);
    }
    std::vector<core::parser::OcrLine> out; out.reserve(acc.size());
    for (auto& la : acc) {
        std::sort(la.words.begin(), la.words.end(), [](const api::tesseract::Word* a, const api::tesseract::Word* b) { if (a->bbox.x != b->bbox.x) return a->bbox.x < b->bbox.x; return a->bbox.y < b->bbox.y; });
        core::parser::OcrLine ol; ol.minY = la.minY; ol.maxY = la.maxY; ol.wordSpans.reserve(la.words.size());
        if (!la.words.empty()) { ol.minX = la.words.front()->bbox.x; ol.maxX = la.words.front()->bbox.x + la.words.front()->bbox.width; }
        else { ol.minX = 0; ol.maxX = 0; }
        std::string txt;
        for (const auto* w : la.words) {
            const int left = w->bbox.x; const int right = w->bbox.x + w->bbox.width; ol.wordSpans.emplace_back(left, right); ol.minX = std::min(ol.minX, left); ol.maxX = std::max(ol.maxX, right);
            if (!txt.empty()) txt.push_back(' '); txt += w->text;
        }
        ol.text = utils::trim(std::move(txt)); out.push_back(std::move(ol));
    }
    return out;
}

ColumnGuess inferColumnModelFromLines(const std::vector<core::parser::OcrLine>& lines, size_t scanN) noexcept {
    ColumnGuess out;
    try {
        const size_t n = std::min(lines.size(), scanN);
        for (size_t i = 0; i < n; ++i) {
            const auto& l = lines[i];
            try { if (out.valutaX < 0) if (auto vx = findTokenCenterX(l, "valuta")) out.valutaX = *vx; }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::inferColumnModelFromLines::valuta", std::current_exception()); }
            try { if (out.debitX < 0) if (auto dx = findPhraseCenterX(l, { "zu","ihren","lasten" })) out.debitX = *dx; }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::inferColumnModelFromLines::debit", std::current_exception()); }
            try { if (out.creditX < 0) if (auto cx = findPhraseCenterX(l, { "zu","ihren","gunsten" })) out.creditX = *cx; }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::inferColumnModelFromLines::credit", std::current_exception()); }
            if (out.valutaX >= 0 && out.debitX >= 0 && out.creditX >= 0) break;
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::inferColumnModelFromLines", std::current_exception()); }
    return out;
}

using detail::RawLineLite;

std::vector<RawLineLite> groupMergeLinesRaw(const std::vector<RawLineLite>& lines, int maxGapPx) {
    if (lines.size() <= 1) return lines;
    std::vector<RawLineLite> merged; merged.reserve(lines.size());
    RawLineLite cur = lines.front();
    for (size_t i = 1; i < lines.size(); ++i) {
        const auto& next = lines[i];
        if (next.minY - cur.maxY <= maxGapPx) {
            cur.maxY = next.maxY; cur.wordSpans.insert(cur.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); cur.text += " " + next.text;
        }
        else { merged.push_back(std::move(cur)); cur = next; }
    }
    merged.push_back(std::move(cur)); return merged;
}

std::vector<RawLineLite> selectiveGroupMergeLinesRaw(const std::vector<RawLineLite>& lines, int maxGapPx, const ColumnGuess& seedCols) {
    if (lines.size() <= 1) return lines;
    std::vector<RawLineLite> merged; merged.reserve(lines.size());
    RawLineLite cur = lines.front();
    for (size_t i = 1; i < lines.size(); ++i) {
        const auto& next = lines[i];
        int gap = next.minY - cur.maxY;
        if (gap <= maxGapPx) {
            bool evidence = false;
            try {
                // convert to OcrLine for helper checks
                core::parser::OcrLine olCur; olCur.minX = cur.minX; olCur.maxX = cur.maxX; olCur.minY = cur.minY; olCur.maxY = cur.maxY; olCur.wordSpans = cur.wordSpans; olCur.text = cur.text;
                core::parser::OcrLine olNext; olNext.minX = next.minX; olNext.maxX = next.maxX; olNext.minY = next.minY; olNext.maxY = next.maxY; olNext.wordSpans = next.wordSpans; olNext.text = next.text;
                if (hasAmountLikeTokenInLine(olCur, seedCols.valutaX) || hasAmountLikeTokenInLine(olNext, seedCols.valutaX)) evidence = true;
                if (!evidence && hasLeftDescriptiveText(olCur, seedCols.valutaX) && hasTokenNearX(olNext, seedCols.valutaX, parserConfig.tokenNearMergeBandPx)) evidence = true;
                if (!evidence && hasLeftDescriptiveText(olNext, seedCols.valutaX) && hasTokenNearX(olCur, seedCols.valutaX, parserConfig.tokenNearMergeBandPx)) evidence = true;
                if (!evidence && isLooseTransactionLine(olCur, seedCols.valutaX)) evidence = true;
                if (!evidence && isLooseTransactionLine(olNext, seedCols.valutaX)) evidence = true;
            }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::selectiveGroupMergeLinesRaw", std::current_exception()); }
            if (evidence) { cur.maxY = next.maxY; cur.wordSpans.insert(cur.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); cur.text += " " + next.text; continue; }
        }
        merged.push_back(std::move(cur)); cur = next;
    }
    merged.push_back(std::move(cur)); return merged;
}

core::parser::OcrLine toOcrLineFromRawWords(const RawLineLite& src, size_t i0, size_t i1) noexcept {
    core::parser::OcrLine out;
    if (i0 >= i1 || i0 >= src.wordSpans.size()) return out;
    i1 = std::min(i1, src.wordSpans.size());
    out.minY = src.minY; out.maxY = src.maxY;
    out.minX = src.wordSpans[i0].first; out.maxX = src.wordSpans[i0].second;
    out.wordSpans.reserve(i1 - i0);
    for (size_t i = i0; i < i1; ++i) {
        const auto& sp = src.wordSpans[i]; out.minX = std::min(out.minX, sp.first); out.maxX = std::max(out.maxX, sp.second); out.wordSpans.push_back(sp);
    }
    const auto toks = utils::splitWhitespace(src.text);
    std::string txt;
    for (size_t i = i0; i < i1 && i < toks.size(); ++i) { if (!txt.empty()) txt.push_back(' '); txt += toks[i]; }
    out.text = utils::trim(std::move(txt));
    return out;
}

core::parser::TransactionMainRow splitMainRowFromRaw(const RawLineLite& src, int valutaX, int debitX, int creditX) noexcept {
    core::parser::TransactionMainRow row;
    const auto toks = utils::splitWhitespace(src.text);
    if (toks.size() != src.wordSpans.size() || toks.empty()) {
        row.left.line.text = src.text;
        row.left.line.minX = src.minX; row.left.line.maxX = src.maxX; row.left.line.minY = src.minY; row.left.line.maxY = src.maxY; row.left.line.wordSpans = src.wordSpans;
        return row;
    }
    auto cxAt = [&](size_t i)->int { const auto& sp = src.wordSpans[i]; return (sp.first + sp.second) / 2; };
    auto idxNearX = [&](int x, int skipIdx)->int { int bestIdx = -1; int bestDist = std::numeric_limits<int>::max(); for (size_t i = 0; i < toks.size(); ++i) { if ((int)i == skipIdx) continue; int d = std::abs(cxAt(i) - x); if (d < bestDist) { bestDist = d; bestIdx = (int)i; } } return bestIdx; };
    auto idxNearXPreferNumeric = [&](int x, int skipIdx)->int { int bestIdx = -1; int bestDist = std::numeric_limits<int>::max(); int bestPriority = -1; for (size_t i = 0; i < toks.size(); ++i) { if ((int)i == skipIdx) continue; int d = std::abs(cxAt(i) - x); int priority = 0; try { if (tokenLooksLikeAmount(toks[i])) priority = 2; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::splitMainRowFromRaw::idxNearXPreferNumeric", std::current_exception()); } if (priority > bestPriority || (priority == bestPriority && d < bestDist)) { bestPriority = priority; bestDist = d; bestIdx = (int)i; } } return bestIdx; };

    int valutaIdx = (valutaX >= 0) ? idxNearX(valutaX, -1) : -1;
    int debitIdx = (debitX >= 0) ? idxNearXPreferNumeric(debitX, -1) : -1;
    int creditIdx = (creditX >= 0) ? idxNearXPreferNumeric(creditX, -1) : -1;
    try {
        if (valutaIdx >= 0 && debitIdx < 0 && creditIdx < 0) {
            int found = -1; for (size_t i = 0; i < toks.size(); ++i) { if (cxAt(i) <= valutaX - 120) continue; try { if (tokenLooksLikeAmount(toks[i])) { found = (int)i; break; } } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::splitMainRowFromRaw::findAmount", std::current_exception()); } }
            if (found >= 0) { if (creditX >= 0) creditIdx = found; else if (debitX >= 0) debitIdx = found; else creditIdx = found; }
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::splitMainRowFromRaw", std::current_exception()); }
    if (valutaIdx >= 0) { if (debitIdx == valutaIdx && debitX >= 0) debitIdx = idxNearX(debitX, valutaIdx); if (creditIdx == valutaIdx && creditX >= 0) creditIdx = idxNearX(creditX, valutaIdx); }
    if (debitIdx >= 0 && creditIdx == debitIdx && creditX >= 0) creditIdx = idxNearX(creditX, debitIdx);

    int firstRight = std::numeric_limits<int>::max(); if (valutaIdx >= 0) firstRight = std::min(firstRight, valutaIdx); if (debitIdx >= 0) firstRight = std::min(firstRight, debitIdx); if (creditIdx >= 0) firstRight = std::min(firstRight, creditIdx); if (firstRight == std::numeric_limits<int>::max()) firstRight = (int)toks.size();

    row.left.line = toOcrLineFromRawWords(src, 0, static_cast<size_t>(std::max(0, firstRight)));
    if (valutaIdx >= 0) { size_t v0 = static_cast<size_t>(valutaIdx); size_t v1 = v0 + 1; if (v1 < toks.size() && std::abs(cxAt(v1) - valutaX) <= parserConfig.valutaNeighborExpandPx) v1 = v1 + 1; row.valuta.line = toOcrLineFromRawWords(src, v0, v1); }
    if (debitIdx >= 0) row.debit.line = toOcrLineFromRawWords(src, static_cast<size_t>(debitIdx), static_cast<size_t>(debitIdx + 1));
    if (creditIdx >= 0) row.credit.line = toOcrLineFromRawWords(src, static_cast<size_t>(creditIdx), static_cast<size_t>(creditIdx + 1));
    return row;
}

core::parser::TransactionMainRow splitMainRowFromOcrLine(const core::parser::OcrLine& src, int valutaX, int debitX, int creditX) noexcept {
    // convert OcrLine to RawLineLite and reuse implementation
    RawLineLite rl; rl.minX = src.minX; rl.maxX = src.maxX; rl.minY = src.minY; rl.maxY = src.maxY; rl.wordSpans = src.wordSpans; rl.text = src.text;
    return splitMainRowFromRaw(rl, valutaX, debitX, creditX);
}

std::optional<std::pair<core::parser::TransactionMainRow, int>> tryVerticalStart(const std::vector<core::parser::OcrLine>& lines, size_t li, const ColumnGuess& cols, const std::string& currentBookingDate) noexcept {
    try {
        if (li >= lines.size()) return std::nullopt;
        const auto& l = lines[li];
        // prev + curr
        if (li > 0) {
            const auto& prev = lines[li - 1];
            // Relaxed: accept amount tokens with a wider band and also consider short-date + amount combos
            int wideBand = std::max(parserConfig.amountNearValutaBandPx, parserConfig.tokenNearMergeBandPx);
            if ((hasLeftDescriptiveText(prev, cols.valutaX) && !hasAmountLikeTokenInLine(prev, cols.valutaX) && hasAmountNearValuta(l, cols.valutaX, wideBand)) ||
                (hasAmountLikeTokenInLine(prev, cols.valutaX) && hasLeftDescriptiveText(l, cols.valutaX))) {
                core::parser::helpers::RawLineLite merged; merged.minX = prev.minX; merged.maxX = l.maxX; merged.minY = prev.minY; merged.maxY = l.maxY; merged.wordSpans = prev.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), l.wordSpans.begin(), l.wordSpans.end()); merged.text = prev.text + std::string(" ") + l.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 0));
            }
            // Additional relaxed case: prev contains a short date token and current contains an amount-like token (within wideBand)
            try {
                if (hasShortDateToken(prev.text) && hasAmountNearValuta(l, cols.valutaX, wideBand)) {
                    core::parser::helpers::RawLineLite merged; merged.minX = prev.minX; merged.maxX = l.maxX; merged.minY = prev.minY; merged.maxY = l.maxY; merged.wordSpans = prev.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), l.wordSpans.begin(), l.wordSpans.end()); merged.text = prev.text + std::string(" ") + l.text;
                    auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                    return std::make_optional(std::make_pair(main, 0));
                }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryVerticalStart::prev", std::current_exception()); }
        }
        // curr + next
        if (li + 1 < lines.size()) {
            const auto& next = lines[li + 1];
            int wideBand2 = std::max(parserConfig.amountNearValutaBandPx, parserConfig.tokenNearMergeBandPx);
            if ((hasLeftDescriptiveText(l, cols.valutaX) && !hasAmountLikeTokenInLine(l, cols.valutaX) && hasAmountNearValuta(next, cols.valutaX, wideBand2)) ||
                (hasAmountLikeTokenInLine(l, cols.valutaX) && hasLeftDescriptiveText(next, cols.valutaX))) {
                core::parser::helpers::RawLineLite merged; merged.minX = l.minX; merged.maxX = next.maxX; merged.minY = l.minY; merged.maxY = next.maxY; merged.wordSpans = l.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); merged.text = l.text + std::string(" ") + next.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 1));
            }
            // Additional relaxed: short-date in current + amount in next
            try {
                if (hasShortDateToken(l.text) && hasAmountNearValuta(next, cols.valutaX, wideBand2)) {
                    core::parser::helpers::RawLineLite merged; merged.minX = l.minX; merged.maxX = next.maxX; merged.minY = l.minY; merged.maxY = next.maxY; merged.wordSpans = l.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); merged.text = l.text + std::string(" ") + next.text;
                    auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                    return std::make_optional(std::make_pair(main, 1));
                }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryVerticalStart::next", std::current_exception()); }
            if (hasAmountLikeTokenInLine(l, cols.valutaX) && hasLeftDescriptiveText(next, cols.valutaX)) {
                core::parser::helpers::RawLineLite merged; merged.minX = l.minX; merged.maxX = next.maxX; merged.minY = l.minY; merged.maxY = next.maxY; merged.wordSpans = l.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); merged.text = l.text + std::string(" ") + next.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 1));
            }
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryVerticalStart", std::current_exception()); }
    return std::nullopt;
}

std::optional<std::pair<core::parser::TransactionMainRow, int>> tryCombinedStart(const std::vector<core::parser::OcrLine>& lines, size_t li, const ColumnGuess& cols) noexcept {
    try {
        if (li >= lines.size()) return std::nullopt;
        const auto& txt = lines[li].text;
        std::string combPrev = txt;
        if (li > 0) combPrev = lines[li - 1].text + std::string(" ") + txt;
        std::string combNext = txt;
        if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li + 1].text;
        try {
            if (!combPrev.empty() && std::regex_search(combPrev, std::regex(R"((
            \d{2}\.\s*\d{2}\)\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?\b))", std::regex::ECMAScript))) {
                auto m = splitMainRowFromOcrLine(lines[li], cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(m, 0));
            }
        }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryCombinedStart::prev", std::current_exception()); }
        try {
            if (!combNext.empty() && std::regex_search(combNext, std::regex(R"((
            \d{2}\.\s*\d{2}\)\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?\b))", std::regex::ECMAScript))) {
                auto m = splitMainRowFromOcrLine(lines[li], cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(m, 1));
            }
        }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryCombinedStart::next", std::current_exception()); }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::tryCombinedStart", std::current_exception()); }
    return std::nullopt;
}

void appendDetailLine(core::parser::TransactionBlock& cur, const core::parser::OcrLine& l, const ColumnGuess& cols, std::vector<std::string>* debugOut) noexcept {
    try {
        if (cols.valutaX >= 0) {
            const auto toks = utils::splitWhitespace(l.text);
            if (toks.size() == l.wordSpans.size()) {
                size_t cut = toks.size();
                for (size_t i = 0; i < toks.size(); ++i) {
                    const auto& sp = l.wordSpans[i];
                    const int cx = (sp.first + sp.second) / 2;
                    if (cx >= cols.valutaX) { cut = i; break; }
                }
                auto leftPart = toOcrLineFromRawWords(detail::RawLineLite{ l.minX,l.maxX,l.minY,l.maxY,l.wordSpans,l.text }, 0, cut);
                if (!utils::trim(leftPart.text).empty()) cur.detailLines.push_back(leftPart);
                if (debugOut) debugOut->push_back(std::string("detail.append.helper\ttext=") + l.text + std::string("\tcut=") + std::to_string(cut));
                return;
            }
        }
        // fallback: push entire line
        cur.detailLines.push_back(l);
        if (debugOut) debugOut->push_back(std::string("detail.append.helper.whole\t") + l.text);
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::appendDetailLine", std::current_exception()); }
}

core::parser::TransactionMainRow handleMainRow(const core::parser::OcrLine& line, const ColumnGuess& cols, bool isGeom, std::vector<std::string>& debugOut) noexcept {
    try {
        auto main = splitMainRowFromOcrLine(line, cols.valutaX, cols.debitX, cols.creditX);
        try {
            std::ostringstream sel; sel << "tx.main.split\tline?\tvalutaX=" << cols.valutaX << "\tdebitX=" << cols.debitX << "\tcreditX=" << cols.creditX;
            debugOut.push_back(sel.str());
        }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::splitLog", std::current_exception()); }
        if (isGeom) {
            try { debugOut.push_back(std::string("tx.main.geom\t") + line.text); }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::geom", std::current_exception()); }
        }
        try { debugOut.push_back(std::string("tx.main\t") + line.text); }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::main", std::current_exception()); }
        if (!main.left.line.text.empty()) try { debugOut.push_back(std::string("tx.main.left\t") + main.left.line.text); }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::left", std::current_exception()); }
        if (!main.valuta.line.text.empty()) try { debugOut.push_back(std::string("tx.main.valuta\t") + main.valuta.line.text); }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::valuta", std::current_exception()); }
        if (!main.debit.line.text.empty()) try { debugOut.push_back(std::string("tx.main.debit\t") + main.debit.line.text); }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::debit", std::current_exception()); }
        if (!main.credit.line.text.empty()) try { debugOut.push_back(std::string("tx.main.credit\t") + main.credit.line.text); }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow::credit", std::current_exception()); }
        return main;
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::handleMainRow", std::current_exception()); }
    return core::parser::TransactionMainRow{};
}

// Header/footer and orphan helpers
std::pair<int, bool> detectHeaderRegion(const std::vector<core::parser::OcrLine>& lines, size_t scanLines, int preferValutaX) {
    int lastBlockY = -1;
    size_t idx = 0;
    int noiseCount = 0;
    for (const auto& l : lines) {
        ++idx;
        if (idx > scanLines) break;
        const auto txt = l.text;
        if (txt.empty()) continue;
        bool isStrongHeaderLike = false;
        bool isNoise = false;
        try {
            core::parser::OcrLine olLine; olLine.minX = l.minX; olLine.maxX = l.maxX; olLine.minY = l.minY; olLine.maxY = l.maxY; olLine.wordSpans = l.wordSpans; olLine.text = l.text;
            if (core::parser::heuristics::isTransactionsSectionHeader(txt)) isStrongHeaderLike = true;
            if (normalizeAlnumLower(txt).find("valuta") != std::string::npos) isStrongHeaderLike = true;
            if (core::parser::heuristics::isDebitCreditHeaderLine(txt)) isStrongHeaderLike = true;
            // Full date commonly appears in header/summary lines (e.g. "Neuer Kontostand vom DD.MM.YYYY 12.345,67").
            // Treat any line containing a full date as header-like to avoid classifying balance summaries as transactions.
            if (findFirstFullDate(txt).has_value()) {
                try {
                    isStrongHeaderLike = true;
                } catch(...) { isStrongHeaderLike = true; }
            }

            if (core::parser::heuristics::isHeaderNoiseLine(txt)) isNoise = true;
        }
        catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::detectHeaderRegion", std::current_exception()); }

        if (isStrongHeaderLike) {
            lastBlockY = std::max(lastBlockY, l.maxY);
            // keep scanning until we hit a non-header-like line
        } else if (isNoise) {
            ++noiseCount;
            if (noiseCount >= 2) {
                lastBlockY = std::max(lastBlockY, l.maxY);
            }
        } else {
            if (lastBlockY >= 0) break;
        }
    }
    return { lastBlockY, lastBlockY >= 0 };
}

std::optional<std::string> findFallbackBookingDate(const std::vector<core::parser::OcrLine>& lines, size_t scanLines) noexcept {
    try {
        const size_t n = std::min(lines.size(), scanLines);
        static const std::regex reDate(R"((\d{2}\.\d{2}\.\d{4}))");
        for (size_t i = 0; i < n; ++i) {
            const auto& l = lines[i]; const auto txt = l.text; if (txt.empty()) continue;
            bool isFoot = false; try { if (core::parser::heuristics::isPostTransactionFootnote(txt)) isFoot = true; }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findFallbackBookingDate::isFoot", std::current_exception()); }
            if (isFoot) continue;
            try {
                std::smatch m;
                std::string combPrev = (i > 0 ? (lines[i - 1].text + std::string(" ") + txt) : txt);
                std::string combNext = (i + 1 < lines.size() ? (txt + std::string(" ") + lines[i + 1].text) : txt);
                if (std::regex_search(txt, m, reDate) || std::regex_search(combPrev, m, reDate) || std::regex_search(combNext, m, reDate)) return m.str(1);
            }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findFallbackBookingDate::regex", std::current_exception()); }
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findFallbackBookingDate", std::current_exception()); }
    return std::nullopt;
}

std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept {
    try {
        const auto trimmed = utils::trim(line);
        const auto norm = normalizeAlnumLower(trimmed);
        // require explicit 'buchungsdat' keyword to consider this a header booking date
        if (norm.find("buchungsdat") == std::string::npos && norm.find("buchungsdatum") == std::string::npos) return std::nullopt;

        // look for a full date in the trimmed text
        try {
            if (auto d = findFirstFullDate(trimmed)) return d;
            // try a compacted version (remove non-digit/dot/space) to handle OCR noise
            std::string compact = trimmed;
            compact.erase(std::remove_if(compact.begin(), compact.end(), [](char c){ return !(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || std::isspace(static_cast<unsigned char>(c))); }), compact.end());
            if (auto d2 = findFirstFullDate(compact)) return d2;
        } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findBookingDateInHeader::inner", std::current_exception()); }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::findBookingDateInHeader", std::current_exception()); }
    return std::nullopt;
}

bool detectEarlyEmptyPage(const std::vector<core::parser::OcrLine>& lines, std::string& outDebug, std::string& foundBookingDate) noexcept {
    try {
        const size_t checkLines = std::min(lines.size(), static_cast<size_t>(parserConfig.headerScanLines));
        int footLike = 0; int amountLikeCount = 0; size_t start = lines.size() > checkLines ? lines.size() - checkLines : 0;
        for (size_t ii = start; ii < lines.size(); ++ii) {
            const auto& t = lines[ii].text; if (t.empty()) continue;
            try { if (core::parser::heuristics::isPostTransactionFootnote(t)) ++footLike; }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::detectEarlyEmptyPage::footnote", std::current_exception()); }
            try { const auto toks = utils::splitWhitespace(t); for (const auto& tk : toks) { try { if (tokenLooksLikeAmount(tk)) { ++amountLikeCount; break; } } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::detectEarlyEmptyPage::tokenLooksLikeAmount", std::current_exception()); } } }
            catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::detectEarlyEmptyPage::amountScan", std::current_exception()); }
        }
        std::ostringstream ss; ss << "page.earlyEmptyCheckBottom\tfootLike=" << footLike << "\tamts=" << amountLikeCount; outDebug = ss.str();
        if (footLike >= static_cast<int>(checkLines * 2 / 3) && amountLikeCount == 0) {
            return true;
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::detectEarlyEmptyPage", std::current_exception()); }
    return false;
}

void attachOrphansToBlocks(std::vector<core::parser::TransactionBlock>& blocks, const std::vector<core::parser::OcrLine>& orphans, int maxGapPx, int valutaX) noexcept {
    try {
        for (const auto& ol : orphans) {
            int bestBlock = -1; int bestGap = std::numeric_limits<int>::max();
            for (size_t bi = 0; bi < blocks.size(); ++bi) {
                const auto& mb = blocks[bi]; if (mb.main.left.empty()) continue;
                const int mBottom = mb.main.left.line.maxY; if (ol.minY >= mBottom) { const int gap = ol.minY - mBottom; if (gap <= maxGapPx && gap < bestGap) { bestGap = gap; bestBlock = static_cast<int>(bi); } }
            }
            if (bestBlock >= 0) {
                bool accept = true; if (valutaX >= 0) { if (ol.maxX >= valutaX + 40) accept = false; }
                if (accept) blocks[static_cast<size_t>(bestBlock)].detailLines.push_back(ol);
            }
        }
    }
    catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::helpers::attachOrphansToBlocks", std::current_exception()); }
}

} // namespace core::parser::helpers} // namespace core::parser::helpers