#include "core/parser/ParserHelpers.h"
#include "core/utils/Util.h"
#include "core/parser/DefaultTransactionParser.h" // for OcrLine
#include "core/parser/DefaultStatementParser.h" // for access to RawLine-like types
#include "core/parser/ParserHeuristics.h"
#include <regex>
#include <sstream>

namespace core::parser::helpers {

bool isAmountLikeToken(const std::string& token) noexcept {
    try {
        static const std::regex amountLike(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?$)");
        return std::regex_match(token, amountLike);
    } catch(...) { return false; }
}

bool containsShortDate(const std::string& text) noexcept {
    try {
        static const std::regex shortDate(R"(\d{2}\.\s*\d{2})");
        return std::regex_search(text, shortDate);
    } catch(...) { return false; }
}

std::optional<std::string> findFirstFullDate(const std::string& text) noexcept {
    try {
        static const std::regex re(R"((\d{2}\.\d{2}\.\d{4}))");
        std::smatch m;
        if (std::regex_search(text, m, re)) return m.str(1);
    } catch(...) {}
    return std::nullopt;
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

        for (size_t j = i; j < toks.size() && (j - i) < 6; ++j) {
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
        const auto toks = utils::splitWhitespace(line.text);
        for (const auto& t : toks) { try { if (isAmountLikeToken(t)) return true; } catch (...) {} }
        if (valutaX >= 0) {
            for (size_t i = 0; i < toks.size() && i < line.wordSpans.size(); ++i) {
                const auto& sp = line.wordSpans[i];
                const int cx = (sp.first + sp.second) / 2;
                if (cx > valutaX - 120) {
                    try { if (isAmountLikeToken(toks[i])) return true; }
                    catch (...) {}
                }
            }
        }
    }
    catch (...) {}
    return false;
}

bool hasLeftDescriptiveText(const core::parser::OcrLine& line, int valutaX) noexcept {
    try {
        const auto toks = utils::splitWhitespace(line.text);
        if (toks.empty()) return false;
        for (size_t i = 0; i < toks.size() && i < line.wordSpans.size(); ++i) {
            const auto& sp = line.wordSpans[i];
            const int cx = (sp.first + sp.second) / 2;
            if (cx < valutaX - 200) {
                for (unsigned char c : toks[i]) if (std::isalpha(c)) return true;
            }
        }
        // fallback: first token contains letters
        for (unsigned char c : toks[0]) if (std::isalpha(c)) return true;
    }
    catch (...) {}
    return false;
}

bool hasAmountNearValuta(const core::parser::OcrLine& line, int valutaX, int bandPx) noexcept {
    if (valutaX < 0) return false;
    try {
        const auto toks = utils::splitWhitespace(line.text);
        if (toks.size() != line.wordSpans.size() || toks.empty()) return false;
        for (size_t i = 0; i < toks.size(); ++i) {
            const auto& sp = line.wordSpans[i];
            const int cx = (sp.first + sp.second) / 2;
            if (std::abs(cx - valutaX) <= bandPx) {
                try { if (isAmountLikeToken(toks[i])) return true; }
                catch (...) {}
            }
        }
    }
    catch (...) {}
    return false;
}

bool isLooseTransactionLine(const core::parser::OcrLine& line, int valutaX) noexcept {
    if (valutaX < 0) return false;
    try {
        if (!containsShortDate(line.text)) return false;
        std::smatch md;
        static const std::regex shortDate(R"((\d{2}\.\s*\d{2}))");
        if (!std::regex_search(line.text, md, shortDate)) return false;
        const size_t datePos = static_cast<size_t>(md.position(0));
        const auto toks = utils::splitWhitespace(line.text);
        bool foundAmountAfterDate = false;
        for (const auto& t : toks) {
            try { if (!isAmountLikeToken(t)) continue; }
            catch (...) { continue; }
            const auto pos = line.text.find(t);
            if (pos != std::string::npos && pos > datePos) { foundAmountAfterDate = true; break; }
        }
        if (!foundAmountAfterDate) return false;
        const auto left = line.text.substr(0, datePos);
        for (unsigned char c : left) { if (std::isalpha(c)) return true; }
    }
    catch (...) {}
    return false;
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
            catch (...) {}
            try { if (out.debitX < 0) if (auto dx = findPhraseCenterX(l, { "zu","ihren","lasten" })) out.debitX = *dx; }
            catch (...) {}
            try { if (out.creditX < 0) if (auto cx = findPhraseCenterX(l, { "zu","ihren","gunsten" })) out.creditX = *cx; }
            catch (...) {}
            if (out.valutaX >= 0 && out.debitX >= 0 && out.creditX >= 0) break;
        }
    }
    catch (...) {}
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
                if (!evidence && hasLeftDescriptiveText(olCur, seedCols.valutaX) && hasTokenNearX(olNext, seedCols.valutaX, 220)) evidence = true;
                if (!evidence && hasLeftDescriptiveText(olNext, seedCols.valutaX) && hasTokenNearX(olCur, seedCols.valutaX, 220)) evidence = true;
                if (!evidence && isLooseTransactionLine(olCur, seedCols.valutaX)) evidence = true;
                if (!evidence && isLooseTransactionLine(olNext, seedCols.valutaX)) evidence = true;
            }
            catch (...) {}
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
    auto idxNearXPreferNumeric = [&](int x, int skipIdx)->int { int bestIdx = -1; int bestDist = std::numeric_limits<int>::max(); int bestPriority = -1; for (size_t i = 0; i < toks.size(); ++i) { if ((int)i == skipIdx) continue; int d = std::abs(cxAt(i) - x); int priority = 0; try { if (isAmountLikeToken(toks[i])) priority = 2; } catch (...) {} if (priority > bestPriority || (priority == bestPriority && d < bestDist)) { bestPriority = priority; bestDist = d; bestIdx = (int)i; } } return bestIdx; };

    int valutaIdx = (valutaX >= 0) ? idxNearX(valutaX, -1) : -1;
    int debitIdx = (debitX >= 0) ? idxNearXPreferNumeric(debitX, -1) : -1;
    int creditIdx = (creditX >= 0) ? idxNearXPreferNumeric(creditX, -1) : -1;
    try {
        if (valutaIdx >= 0 && debitIdx < 0 && creditIdx < 0) {
            int found = -1; for (size_t i = 0; i < toks.size(); ++i) { if (cxAt(i) <= valutaX - 120) continue; try { if (isAmountLikeToken(toks[i])) { found = (int)i; break; } } catch (...) {} }
            if (found >= 0) { if (creditX >= 0) creditIdx = found; else if (debitX >= 0) debitIdx = found; else creditIdx = found; }
        }
    }
    catch (...) {}
    if (valutaIdx >= 0) { if (debitIdx == valutaIdx && debitX >= 0) debitIdx = idxNearX(debitX, valutaIdx); if (creditIdx == valutaIdx && creditX >= 0) creditIdx = idxNearX(creditX, valutaIdx); }
    if (debitIdx >= 0 && creditIdx == debitIdx && creditX >= 0) creditIdx = idxNearX(creditX, debitIdx);

    int firstRight = std::numeric_limits<int>::max(); if (valutaIdx >= 0) firstRight = std::min(firstRight, valutaIdx); if (debitIdx >= 0) firstRight = std::min(firstRight, debitIdx); if (creditIdx >= 0) firstRight = std::min(firstRight, creditIdx); if (firstRight == std::numeric_limits<int>::max()) firstRight = (int)toks.size();

    row.left.line = toOcrLineFromRawWords(src, 0, static_cast<size_t>(std::max(0, firstRight)));
    if (valutaIdx >= 0) { size_t v0 = static_cast<size_t>(valutaIdx); size_t v1 = v0 + 1; if (v1 < toks.size() && std::abs(cxAt(v1) - valutaX) <= 140) v1 = v1 + 1; row.valuta.line = toOcrLineFromRawWords(src, v0, v1); }
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
            if (hasLeftDescriptiveText(prev, cols.valutaX) && !hasAmountLikeTokenInLine(prev, cols.valutaX) && hasAmountNearValuta(l, cols.valutaX, 140)) {
                core::parser::helpers::RawLineLite merged; merged.minX = prev.minX; merged.maxX = l.maxX; merged.minY = prev.minY; merged.maxY = l.maxY; merged.wordSpans = prev.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), l.wordSpans.begin(), l.wordSpans.end()); merged.text = prev.text + std::string(" ") + l.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 0));
            }
            if (hasAmountLikeTokenInLine(prev, cols.valutaX) && hasLeftDescriptiveText(l, cols.valutaX)) {
                core::parser::helpers::RawLineLite merged; merged.minX = prev.minX; merged.maxX = l.maxX; merged.minY = prev.minY; merged.maxY = l.maxY; merged.wordSpans = prev.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), l.wordSpans.begin(), l.wordSpans.end()); merged.text = prev.text + std::string(" ") + l.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 0));
            }
        }
        // curr + next
        if (li + 1 < lines.size()) {
            const auto& next = lines[li + 1];
            if (hasLeftDescriptiveText(l, cols.valutaX) && !hasAmountLikeTokenInLine(l, cols.valutaX) && hasAmountNearValuta(next, cols.valutaX, 140)) {
                core::parser::helpers::RawLineLite merged; merged.minX = l.minX; merged.maxX = next.maxX; merged.minY = l.minY; merged.maxY = next.maxY; merged.wordSpans = l.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); merged.text = l.text + std::string(" ") + next.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 1));
            }
            if (hasAmountLikeTokenInLine(l, cols.valutaX) && hasLeftDescriptiveText(next, cols.valutaX)) {
                core::parser::helpers::RawLineLite merged; merged.minX = l.minX; merged.maxX = next.maxX; merged.minY = l.minY; merged.maxY = next.maxY; merged.wordSpans = l.wordSpans; merged.wordSpans.insert(merged.wordSpans.end(), next.wordSpans.begin(), next.wordSpans.end()); merged.text = l.text + std::string(" ") + next.text;
                auto main = splitMainRowFromRaw(merged, cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(main, 1));
            }
        }
    }
    catch (...) {}
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
        catch (...) {}
        try {
            if (!combNext.empty() && std::regex_search(combNext, std::regex(R"((
            \d{2}\.\s*\d{2}\)\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?\b))", std::regex::ECMAScript))) {
                auto m = splitMainRowFromOcrLine(lines[li], cols.valutaX, cols.debitX, cols.creditX);
                return std::make_optional(std::make_pair(m, 1));
            }
        }
        catch (...) {}
    }
    catch (...) {}
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
    catch (...) {}
}

core::parser::TransactionMainRow handleMainRow(const core::parser::OcrLine& line, const ColumnGuess& cols, bool isGeom, std::vector<std::string>& debugOut) noexcept {
    try {
        auto main = splitMainRowFromOcrLine(line, cols.valutaX, cols.debitX, cols.creditX);
        try {
            std::ostringstream sel; sel << "tx.main.split\tline?\tvalutaX=" << cols.valutaX << "\tdebitX=" << cols.debitX << "\tcreditX=" << cols.creditX;
            debugOut.push_back(sel.str());
        }
        catch (...) {}
        if (isGeom) {
            try { debugOut.push_back(std::string("tx.main.geom\t") + line.text); }
            catch (...) {}
        }
        try { debugOut.push_back(std::string("tx.main\t") + line.text); }
        catch (...) {}
        if (!main.left.line.text.empty()) try { debugOut.push_back(std::string("tx.main.left\t") + main.left.line.text); }
        catch (...) {}
        if (!main.valuta.line.text.empty()) try { debugOut.push_back(std::string("tx.main.valuta\t") + main.valuta.line.text); }
        catch (...) {}
        if (!main.debit.line.text.empty()) try { debugOut.push_back(std::string("tx.main.debit\t") + main.debit.line.text); }
        catch (...) {}
        if (!main.credit.line.text.empty()) try { debugOut.push_back(std::string("tx.main.credit\t") + main.credit.line.text); }
        catch (...) {}
        return main;
    }
    catch (...) {}
    return core::parser::TransactionMainRow{};
}

// Header/footer and orphan helpers
std::pair<int, bool> detectHeaderRegion(const std::vector<core::parser::OcrLine>& lines, size_t scanLines) {
    int lastBlockY = -1;
    size_t idx = 0;
    for (const auto& l : lines) {
        ++idx;
        if (idx > scanLines) break;
        const auto txt = l.text;
        if (txt.empty()) continue;
        bool isHeaderLike = false;
        try {
            if (core::parser::heuristics::isTransactionsSectionHeader(txt) || normalizeAlnumLower(txt).find("valuta") != std::string::npos || core::parser::heuristics::isDebitCreditHeaderLine(txt) || findFirstFullDate(txt).has_value() || core::parser::heuristics::isHeaderNoiseLine(txt)) isHeaderLike = true;
        }
        catch (...) {}
        if (isHeaderLike) lastBlockY = std::max(lastBlockY, l.maxY);
        else { if (lastBlockY >= 0) break; }
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
            catch (...) {}
            if (isFoot) continue;
            try {
                std::smatch m;
                std::string combPrev = (i > 0 ? (lines[i - 1].text + std::string(" ") + txt) : txt);
                std::string combNext = (i + 1 < lines.size() ? (txt + std::string(" ") + lines[i + 1].text) : txt);
                if (std::regex_search(txt, m, reDate) || std::regex_search(combPrev, m, reDate) || std::regex_search(combNext, m, reDate)) return m.str(1);
            }
            catch (...) {}
        }
    }
    catch (...) {}
    return std::nullopt;
}

std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept {
    try {
        const auto trimmed = utils::trim(line);
        const auto lower = utils::lowerAscii(trimmed);
        const auto keyPos = lower.find("buchungsdat");
        if (keyPos == std::string::npos) return std::nullopt;
        const auto afterKey = trimmed.substr(keyPos);
        if (auto d = findFirstFullDate(afterKey)) return d;
        if (auto d2 = findFirstFullDate(trimmed)) return d2;
    } catch(...) {}
    return std::nullopt;
}

bool detectEarlyEmptyPage(const std::vector<core::parser::OcrLine>& lines, std::string& outDebug, std::string& foundBookingDate) noexcept {
    try {
        const size_t checkLines = std::min(lines.size(), static_cast<size_t>(12));
        int footLike = 0; int amountLikeCount = 0; size_t start = lines.size() > checkLines ? lines.size() - checkLines : 0;
        for (size_t ii = start; ii < lines.size(); ++ii) {
            const auto& t = lines[ii].text; if (t.empty()) continue;
            try { if (core::parser::heuristics::isPostTransactionFootnote(t)) ++footLike; }
            catch (...) {}
            try { const auto toks = utils::splitWhitespace(t); for (const auto& tk : toks) { try { if (core::parser::helpers::isAmountLikeToken(tk)) { ++amountLikeCount; break; } } catch (...) {} } }
            catch (...) {}
        }
        std::ostringstream ss; ss << "page.earlyEmptyCheckBottom\tfootLike=" << footLike << "\tamts=" << amountLikeCount; outDebug = ss.str();
        if (footLike >= static_cast<int>(checkLines * 2 / 3) && amountLikeCount == 0) {
            return true;
        }
    }
    catch (...) {}
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
    catch (...) {}
}

} // namespace core::parser::helpers