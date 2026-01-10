#include "core/pch.h"

#include "core/parser/DefaultStatementParser.h"

#include "core/parser/DefaultTransactionParser.h"
#include "core/utils/Util.h"
#include "core/utils/UniqId.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>

using core::parser::DefaultTransactionParser;
using core::parser::OcrCell;
using core::parser::OcrLine;
using core::parser::TransactionBlock;
using core::parser::TransactionMainRow;
using utils::lowerAscii;
using utils::trim;

namespace {

static std::string normalizeAlnumLower(const std::string& s) {
    std::string o;
    o.reserve(s.size());
    for (unsigned char c : utils::lowerAscii(s)) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) o.push_back(static_cast<char>(c));
    }
    return o;
}

static bool isValutaHeaderLine(const std::string& line) {
    const auto n = normalizeAlnumLower(line);
    return n.find("valuta") != std::string::npos;
}

struct RawLine {
    int cy = 0;
    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

struct ColumnModel {
    int valutaX = -1;
    int debitX = -1;
    int creditX = -1;
    int valutaCol = -1;
    int debitCol = -1;
    int creditCol = -1;
    bool hasValuta() const { return valutaX >= 0; }
    bool hasDebit() const { return debitX >= 0; }
    bool hasCredit() const { return creditX >= 0; }
};

struct RowModel {
    bool inSection = false;
};

static bool hasTokenNearX(const RawLine& l, int x, int bandPx) {
    if (x < 0) return false;
    const auto toks = utils::splitWhitespace(l.text);
    if (toks.size() != l.wordSpans.size() || toks.empty()) return false;

    for (size_t i = 0; i < toks.size(); ++i) {
        const auto& sp = l.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (std::abs(cx - x) <= bandPx) return true;
    }
    return false;
}

static bool isLikelyTransactionMainRowGeom(const RawLine& l, const ColumnModel& cols) {
    if (!cols.hasValuta() || (!cols.hasDebit() && !cols.hasCredit())) return false;
    const auto toks = utils::splitWhitespace(l.text);
    if (toks.size() != l.wordSpans.size() || toks.size() < 3) return false;

    const int band = 140;
    const bool hasValuta = hasTokenNearX(l, cols.valutaX, band);
    const bool hasDebit = cols.hasDebit() ? hasTokenNearX(l, cols.debitX, band) : false;
    const bool hasCredit = cols.hasCredit() ? hasTokenNearX(l, cols.creditX, band) : false;

    bool hasLeft = false;
    for (size_t i = 0; i < toks.size(); ++i) {
        const auto& sp = l.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (cx < cols.valutaX - 200) { hasLeft = true; break; }
    }

    return hasLeft && hasValuta && (hasDebit || hasCredit);
}

static std::vector<RawLine> buildOcrLines(const std::vector<api::tesseract::Word>& words) {
    struct W {
        const api::tesseract::Word* w;
        int cy;
        int top;
        int bottom;
        int left;
    };

    std::vector<W> ws;
    ws.reserve(words.size());
    for (const auto& w : words) {
        bool anyNonSpace = false;
        for (unsigned char c : w.text) {
            if (!std::isspace(c)) { anyNonSpace = true; break; }
        }
        if (!anyNonSpace) continue;

        const int cy = w.bbox.y + w.bbox.height / 2;
        ws.push_back(W{ &w, cy, w.bbox.y, w.bbox.y + w.bbox.height, w.bbox.x });
    }

    std::sort(ws.begin(), ws.end(), [](const W& a, const W& b) {
        if (a.top != b.top) return a.top < b.top;
        if (a.cy != b.cy) return a.cy < b.cy;
        return a.left < b.left;
    });

    int avgH = 0;
    for (const auto& ww : ws) avgH += (ww.bottom - ww.top);
    avgH = ws.empty() ? 10 : std::max(1, avgH / static_cast<int>(ws.size()));
    const int minOverlap = std::max(2, avgH / 3);

    struct LineAcc {
        int cy = 0;
        int minX = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int minY = std::numeric_limits<int>::max();
        int maxY = std::numeric_limits<int>::min();
        std::vector<const api::tesseract::Word*> words;
    };

    std::vector<LineAcc> acc;
    acc.reserve(std::max<size_t>(8, words.size() / 8));

    for (const auto& ww : ws) {
        const auto& w = *ww.w;
        const int wMinX = w.bbox.x;
        const int wMaxX = w.bbox.x + w.bbox.width;
        const int wMinY = w.bbox.y;
        const int wMaxY = w.bbox.y + w.bbox.height;

        int bestIdx = -1;
        int bestOverlap = -1;
        for (int i = static_cast<int>(acc.size()) - 1; i >= 0; --i) {
            const auto& l = acc[static_cast<size_t>(i)];
            const int top = std::max(l.minY, wMinY);
            const int bot = std::min(l.maxY, wMaxY);
            const int ov = bot - top;
            if (ov >= minOverlap && ov > bestOverlap) {
                bestOverlap = ov;
                bestIdx = i;
            }
            if (wMinY - l.maxY > avgH) break;
        }

        if (bestIdx < 0) {
            LineAcc nl;
            nl.cy = ww.cy;
            nl.minX = wMinX;
            nl.maxX = wMaxX;
            nl.minY = wMinY;
            nl.maxY = wMaxY;
            nl.words.push_back(&w);
            acc.push_back(std::move(nl));
            continue;
        }

        auto& l = acc[static_cast<size_t>(bestIdx)];
        l.cy = (l.cy + ww.cy) / 2;
        l.minX = std::min(l.minX, wMinX);
        l.maxX = std::max(l.maxX, wMaxX);
        l.minY = std::min(l.minY, wMinY);
        l.maxY = std::max(l.maxY, wMaxY);
        l.words.push_back(&w);
    }

    std::vector<RawLine> out;
    out.reserve(acc.size());

    for (auto& l : acc) {
        std::sort(l.words.begin(), l.words.end(), [](const api::tesseract::Word* a, const api::tesseract::Word* b) {
            if (a->bbox.x != b->bbox.x) return a->bbox.x < b->bbox.x;
            return a->bbox.y < b->bbox.y;
        });

        RawLine ol;
        ol.cy = l.cy;
        ol.minX = l.minX;
        ol.maxX = l.maxX;
        ol.minY = l.minY;
        ol.maxY = l.maxY;

        ol.wordSpans.reserve(l.words.size());
        for (const auto* w : l.words) {
            const int left = w->bbox.x;
            const int right = w->bbox.x + w->bbox.width;
            ol.wordSpans.emplace_back(left, right);
        }

        for (const auto* w : l.words) {
            if (!ol.text.empty()) ol.text.push_back(' ');
            ol.text += w->text;
        }

        ol.text = trim(std::move(ol.text));
        out.push_back(std::move(ol));
    }

    return out;
}

static bool containsAny(const std::string& hayLower, std::initializer_list<const char*> needlesLower) {
    for (auto n : needlesLower) {
        if (hayLower.find(n) != std::string::npos) return true;
    }
    return false;
}

static bool isHeaderNoiseLine(const std::string& line) {
    const auto l = lowerAscii(line);
    if (l.size() <= 2) return true;

    return containsAny(l, {
        "kontoauszug",
        "auszug-nr",
        "seite-nr",
        "iban",
        "bic",
        "commerzbank",
        "ust-idnr",
        "hamburg",
        "ihr ansprechpartner",
        "telefonnummer",
        "kontowährung",
    });
}

static bool isFooterLine(const std::string& line) {
    const auto l = lowerAscii(line);
    return containsAny(l, { "folgeseite", "vsa000", "fil200", "ktea", "folgende seite" });
}

static bool isPostTransactionFootnote(const std::string& line) {
    const auto l = lowerAscii(line);
    if (!l.empty() && l.find("kontostand") != std::string::npos) return false;

    return containsAny(l, {
        "guthaben sind als",
        "einlagensicherungsgesetzes",
        "informationsbogen",
        "der angegebene kontostand",
        "wertstellung der buchungen",
        "kontoüberziehung",
        "neuer kontostand",
        "alter kontostand",
        "folgeseite",
    });
}

static bool isTransactionsSectionHeader(const std::string& line) {
    const auto l = lowerAscii(line);
    if (l.find("angaben zu den umsätzen") != std::string::npos) return true;
    const bool hasValuta = (l.find("valuta") != std::string::npos);
    const bool hasUmsaetze = (l.find("umsätzen") != std::string::npos) || (l.find("umsaetzen") != std::string::npos);
    const bool hasAngaben = (l.find("angaben") != std::string::npos);
    return hasValuta && (hasUmsaetze || hasAngaben);
}

static bool isDebitCreditHeaderLine(const std::string& line) {
    const auto n = normalizeAlnumLower(line);
    return (n.find("zuihrenlasten") != std::string::npos) &&
           (n.find("zuihrengunsten") != std::string::npos);
}

static std::optional<int> findTokenCenterX(const RawLine& l, const std::string& tokenLower) {
    const auto toks = utils::splitWhitespace(l.text);
    if (toks.size() != l.wordSpans.size()) return std::nullopt;

    const auto want = normalizeAlnumLower(tokenLower);
    for (size_t i = 0; i < toks.size(); ++i) {
        if (normalizeAlnumLower(toks[i]) != want) continue;
        const auto& sp = l.wordSpans[i];
        return (sp.first + sp.second) / 2;
    }

    for (size_t i = 0; i < toks.size(); ++i) {
        const auto n = normalizeAlnumLower(toks[i]);
        if (n.find(want) == std::string::npos) continue;
        const auto& sp = l.wordSpans[i];
        return (sp.first + sp.second) / 2;
    }

    return std::nullopt;
}

static std::optional<int> findPhraseCenterX(const RawLine& l, std::initializer_list<const char*> phraseLower) {
    const auto toks = utils::splitWhitespace(l.text);
    if (toks.size() != l.wordSpans.size()) return std::nullopt;

    std::string want;
    for (auto w : phraseLower) want += normalizeAlnumLower(std::string(w));
    if (want.empty() || toks.empty()) return std::nullopt;

    for (size_t i = 0; i < toks.size(); ++i) {
        std::string acc;
        int left = l.wordSpans[i].first;
        int right = l.wordSpans[i].second;

        for (size_t j = i; j < toks.size() && (j - i) < 6; ++j) {
            acc += normalizeAlnumLower(toks[j]);
            right = l.wordSpans[j].second;
            if (acc.find(want) != std::string::npos) {
                return (left + right) / 2;
            }
        }
    }

    return std::nullopt;
}

static std::optional<std::string> extractBookingDateFromHeader(const std::string& line) {
    const auto l = lowerAscii(trim(line));
    if (l.rfind("buchungsdatum", 0) != 0) return std::nullopt;

    static const std::regex re(R"(\b(\d{2}\.\d{2}\.\d{4})\b)");
    std::sregex_iterator it(line.begin(), line.end(), re);
    std::sregex_iterator end;
    if (it == end) return std::nullopt;
    const auto first = (*it).str(1);
    ++it;
    if (it != end) return std::nullopt;
    return first;
}

static bool isLikelyTransactionMainRowText(const std::string& line) {
    static const std::regex re(R"((\d{2}\.\d{2})\s+\d{1,3}(?:[\.,]\d{3})*,\d{2}-?(?:\s+.*)?\s*$)");
    return std::regex_search(line, re);
}

static OcrLine toOcrLineFromWords(const RawLine& src, size_t i0, size_t i1) {
    OcrLine out;
    if (i0 >= i1 || i0 >= src.wordSpans.size()) return out;
    i1 = std::min(i1, src.wordSpans.size());

    out.minY = src.minY;
    out.maxY = src.maxY;

    out.minX = src.wordSpans[i0].first;
    out.maxX = src.wordSpans[i0].second;
    out.wordSpans.reserve(i1 - i0);

    for (size_t i = i0; i < i1; ++i) {
        const auto& sp = src.wordSpans[i];
        out.minX = std::min(out.minX, sp.first);
        out.maxX = std::max(out.maxX, sp.second);
        out.wordSpans.push_back(sp);
    }

    const auto toks = utils::splitWhitespace(src.text);
    std::string txt;
    for (size_t i = i0; i < i1 && i < toks.size(); ++i) {
        if (!txt.empty()) txt.push_back(' ');
        txt += toks[i];
    }
    out.text = trim(std::move(txt));
    return out;
}

static TransactionMainRow splitMainRow(const RawLine& src, const ColumnModel& cols) {
    TransactionMainRow row;

    const auto toks = utils::splitWhitespace(src.text);
    if (toks.size() != src.wordSpans.size() || toks.empty()) {
        row.left.line.text = src.text;
        row.left.line.minX = src.minX;
        row.left.line.maxX = src.maxX;
        row.left.line.minY = src.minY;
        row.left.line.maxY = src.maxY;
        row.left.line.wordSpans = src.wordSpans;
        return row;
    }

    auto cxAt = [&](size_t i) -> int {
        const auto& sp = src.wordSpans[i];
        return (sp.first + sp.second) / 2;
    };

    auto idxNearX = [&](int x, int skipIdx) -> int {
        int bestIdx = -1;
        int bestDist = std::numeric_limits<int>::max();
        for (size_t i = 0; i < toks.size(); ++i) {
            if (static_cast<int>(i) == skipIdx) continue;
            const int cx = cxAt(i);
            const int d = std::abs(cx - x);
            if (d < bestDist) { bestDist = d; bestIdx = static_cast<int>(i); }
        }
        return bestIdx;
    };

    // Helper: prefer indices whose token looks like an amount
    auto idxNearXPreferNumeric = [&](int x, int skipIdx) -> int {
        int bestIdx = -1;
        int bestDist = std::numeric_limits<int>::max();
        int bestPriority = -1;
        static const std::regex amountLike(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?$)");
        for (size_t i = 0; i < toks.size(); ++i) {
            if (static_cast<int>(i) == skipIdx) continue;
            const int cx = cxAt(i);
            const int d = std::abs(cx - x);
            int priority = 0;
            try { if (std::regex_match(toks[i], amountLike)) priority = 2; }
            catch(...) {}
            // prefer numeric tokens even if slightly farther
            if (priority > bestPriority || (priority == bestPriority && d < bestDist)) {
                bestPriority = priority;
                bestDist = d;
                bestIdx = static_cast<int>(i);
            }
        }
        return bestIdx;
    };

    int valutaIdx = cols.hasValuta() ? idxNearX(cols.valutaX, -1) : -1;
    int debitIdx = cols.hasDebit() ? idxNearXPreferNumeric(cols.debitX, -1) : -1;
    int creditIdx = cols.hasCredit() ? idxNearXPreferNumeric(cols.creditX, -1) : -1;

    // If assigned token looks like a short date (valuta), avoid using it as amount token.
    try {
        static const std::regex shortDate(R"(^\(?\d{2}\.\d{2}\)?$)");
        static const std::regex amountLike(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?$)");
        if (creditIdx >= 0) {
            const auto &t = toks[static_cast<size_t>(creditIdx)];
            if (std::regex_match(t, shortDate)) {
                // try to find alternative numeric token near creditX
                int alt = idxNearXPreferNumeric(cols.creditX, valutaIdx);
                if (alt >= 0 && std::regex_match(toks[static_cast<size_t>(alt)], amountLike)) {
                    creditIdx = alt;
                } else {
                    // clear credit index to avoid misinterpretation
                    creditIdx = -1;
                }
            }
        }
        if (debitIdx >= 0) {
            const auto &t = toks[static_cast<size_t>(debitIdx)];
            if (std::regex_match(t, shortDate)) {
                int alt = idxNearXPreferNumeric(cols.debitX, valutaIdx);
                if (alt >= 0 && std::regex_match(toks[static_cast<size_t>(alt)], amountLike)) {
                    debitIdx = alt;
                } else {
                    debitIdx = -1;
                }
            }
        }
    } catch (...) {}

    if (valutaIdx >= 0) {
        if (debitIdx == valutaIdx && cols.hasDebit()) debitIdx = idxNearX(cols.debitX, valutaIdx);
        if (creditIdx == valutaIdx && cols.hasCredit()) creditIdx = idxNearX(cols.creditX, valutaIdx);
    }
    if (debitIdx >= 0) {
        if (creditIdx == debitIdx && cols.hasCredit()) creditIdx = idxNearX(cols.creditX, debitIdx);
    }

    int firstRight = std::numeric_limits<int>::max();
    if (valutaIdx >= 0) firstRight = std::min(firstRight, valutaIdx);
    if (debitIdx >= 0) firstRight = std::min(firstRight, debitIdx);
    if (creditIdx >= 0) firstRight = std::min(firstRight, creditIdx);
    if (firstRight == std::numeric_limits<int>::max()) firstRight = static_cast<int>(toks.size());

    row.left.line = toOcrLineFromWords(src, 0, static_cast<size_t>(std::max(0, firstRight)));

    if (valutaIdx >= 0) {
        size_t v0 = static_cast<size_t>(valutaIdx);
        size_t v1 = v0 + 1;
        if (cols.hasValuta() && v1 < toks.size()) {
            const int cx1 = cxAt(v1);
            if (std::abs(cx1 - cols.valutaX) <= 140) v1 = v1 + 1;
        }
        row.valuta.line = toOcrLineFromWords(src, v0, v1);
    }
    if (debitIdx >= 0) row.debit.line = toOcrLineFromWords(src, static_cast<size_t>(debitIdx), static_cast<size_t>(debitIdx + 1));
    if (creditIdx >= 0) row.credit.line = toOcrLineFromWords(src, static_cast<size_t>(creditIdx), static_cast<size_t>(creditIdx + 1));

    return row;
}

} // namespace

DefaultStatementParser::ParseResult DefaultStatementParser::parse(const api::opencv::Table& /*table*/,
                                                                  const api::tesseract::ExtractResult& ocr,
                                                                  const std::string& pageCropImagePath,
                                                                  std::shared_ptr<api::opencv::IOpenCvService> opencv,
                                                                  std::string initialBookingDate,
                                                                  int initialTransactionIndex) {
    ParseResult out;

    out.debugLines.push_back(std::string("pageCropImagePath\t") + pageCropImagePath);
    out.debugLines.push_back(std::string("initialBookingDate\t") + initialBookingDate);
    out.debugLines.push_back(std::string("initialTransactionIndex\t") + std::to_string(initialTransactionIndex));

    const auto lines = buildOcrLines(ocr.words);
    out.debugLines.push_back(std::string("ocr.words\t") + std::to_string(ocr.words.size()));
    out.debugLines.push_back(std::string("ocr.lines\t") + std::to_string(lines.size()));

    for (size_t li = 0; li < lines.size(); ++li) {
        const auto& ln = lines[li];
        std::ostringstream lns;
        lns << "line." << li << "\tminX=" << ln.minX << "\tmaxX=" << ln.maxX << "\tminY=" << ln.minY << "\tmaxY=" << ln.maxY << "\ttext=" << ln.text;
        out.debugLines.push_back(lns.str());

        const auto toks = utils::splitWhitespace(ln.text);
        if (!toks.empty() && toks.size() == ln.wordSpans.size()) {
            std::ostringstream tks;
            tks << "line." << li << ".tokens";
            for (size_t ti = 0; ti < toks.size(); ++ti) {
                const auto& sp = ln.wordSpans[ti];
                const int cx = (sp.first + sp.second) / 2;
                tks << " \"" << toks[ti] << "\"@" << cx;
            }
            out.debugLines.push_back(tks.str());
        }
    }

    std::string currentBookingDate = std::move(initialBookingDate);
    int txIndex = std::max(1, initialTransactionIndex);

    std::vector<TransactionBlock> blocks;
    TransactionBlock cur;
    cur.bookingDateGroup = currentBookingDate;

    bool inTransactions = !currentBookingDate.empty();
    ColumnModel cols;
    RowModel rows;

    // Try to initialize column centers from OCR table cells (if present)
    try {
        if (!ocr.tables.empty() && ocr.tables[0].cells.size() > 0) {
            const auto& t = ocr.tables[0];
            // accumulate per-column centers and numeric counts
            std::vector<int> colCounts(static_cast<size_t>(t.cols), 0);
            std::vector<int> colCenterX(static_cast<size_t>(t.cols), 0);
            std::vector<int> colItems(static_cast<size_t>(t.cols), 0);
            for (const auto& c : t.cells) {
                if (c.col >= 0 && c.col < t.cols) {
                    int cx = c.bbox.x + c.bbox.width / 2;
                    colCenterX[static_cast<size_t>(c.col)] += cx;
                    colItems[static_cast<size_t>(c.col)] += 1;
                    // simple numeric heuristic for cell text
                    std::string txt = c.text;
                    bool numeric = false;
                    try {
                        static const std::regex rnum(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{2}\)?-?$)");
                        if (std::regex_search(txt, rnum)) numeric = true;
                    } catch (...) {}
                    if (numeric) colCounts[static_cast<size_t>(c.col)] += 1;
                }
            }

            // compute average centers
            for (int ci = 0; ci < t.cols; ++ci) {
                if (colItems[static_cast<size_t>(ci)] > 0) colCenterX[static_cast<size_t>(ci)] /= colItems[static_cast<size_t>(ci)];
            }

            // heuristics: pick rightmost two numeric-rich columns as valuta/debit/credit
            int bestCol = -1; int bestCount = -1;
            for (int ci = 0; ci < t.cols; ++ci) {
                if (colCounts[static_cast<size_t>(ci)] > bestCount) {
                    bestCount = colCounts[static_cast<size_t>(ci)];
                    bestCol = ci;
                }
            }
            if (bestCol >= 0) {
                // assume bestCol is credit-like (rightmost numeric) and previous is debit
                // find rightmost non-empty column index
                int rightmost = -1;
                for (int ci = t.cols - 1; ci >= 0; --ci) {
                    if (colItems[static_cast<size_t>(ci)] > 0) { rightmost = ci; break; }
                }
                if (rightmost >= 0) {
                    cols.creditX = colCenterX[static_cast<size_t>(rightmost)];
                    cols.creditCol = rightmost;
                    // try previous
                    if (rightmost - 1 >= 0 && colItems[static_cast<size_t>(rightmost - 1)] > 0) {
                        cols.debitX = colCenterX[static_cast<size_t>(rightmost - 1)];
                        cols.debitCol = rightmost - 1;
                    }
                }
                // also try to find a valuta-like column by scanning leftwards for date-like tokens
                for (int ci = 0; ci < t.cols; ++ci) {
                    if (colItems[static_cast<size_t>(ci)] > 0) {
                        // approximate x and set valutaX once found left of debit/credit
                        if (cols.debitCol < 0) {
                            cols.valutaX = colCenterX[static_cast<size_t>(ci)];
                            cols.valutaCol = ci;
                            break;
                        }
                    }
                }

                out.debugLines.push_back(std::string("table.cols") + "\tcols=" + std::to_string(t.cols) + "\trightmost=" + std::to_string(bestCol));
                out.debugLines.push_back(std::string("cols.from_table\t") + "valutaX=" + std::to_string(cols.valutaX) + "\tdebitX=" + std::to_string(cols.debitX) + "\tcreditX=" + std::to_string(cols.creditX)
                                     + "\tvalutaCol=" + std::to_string(cols.valutaCol) + "\tdebitCol=" + std::to_string(cols.debitCol) + "\tcreditCol=" + std::to_string(cols.creditCol));
            }
        }
    } catch (...) {}

    const auto flush = [&]() {
        if (cur.main.left.empty() && cur.detailLines.empty()) return;
        blocks.push_back(std::move(cur));
        cur = TransactionBlock{};
        cur.bookingDateGroup = currentBookingDate;
    };

    std::string prevLine;

    for (size_t li = 0; li < lines.size(); ++li) {
        const auto& l = lines[li];
        const auto txt = l.text;
        if (txt.empty()) continue;

        if (isFooterLine(txt)) { out.debugLines.push_back(std::string("stop.footer\t") + txt + "\tline=" + std::to_string(li)); break; }
        if (isPostTransactionFootnote(txt)) { out.debugLines.push_back(std::string("stop.footnote\t") + txt + "\tline=" + std::to_string(li)); break; }

        if ((!cols.hasDebit() || !cols.hasCredit()) && isDebitCreditHeaderLine(txt)) {
            if (!cols.hasDebit()) {
                if (auto dx = findPhraseCenterX(l, {"zu","ihren","lasten"})) cols.debitX = *dx;
            }
            if (!cols.hasCredit()) {
                if (auto cx = findPhraseCenterX(l, {"zu","ihren","gunsten"})) cols.creditX = *cx;
            }
            out.debugLines.push_back(std::string("header.debitcredit\t") + txt + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.debitX\t") + std::to_string(cols.debitX) + "\tcols.creditX\t" + std::to_string(cols.creditX));
            prevLine = txt;
            continue;
        }

        const auto combined = prevLine.empty() ? txt : (prevLine + " " + txt);
        if (!rows.inSection && (isTransactionsSectionHeader(txt) || isValutaHeaderLine(txt) || isTransactionsSectionHeader(combined))) {
            rows.inSection = true;
            if (!cols.hasValuta()) {
                if (auto vx = findTokenCenterX(l, "valuta")) cols.valutaX = *vx;
            }
            out.debugLines.push_back(std::string("header.section\t") + combined + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.valutaX\t") + std::to_string(cols.valutaX));
            prevLine = txt;
            continue;
        }

        if (auto bd = extractBookingDateFromHeader(txt)) {
            currentBookingDate = *bd;
            out.debugLines.push_back(std::string("header.bookingDate\t") + currentBookingDate + "\tline=" + std::to_string(li));
            flush();
            cur.bookingDateGroup = currentBookingDate;
            prevLine = txt;
            continue;
        }

        if (!inTransactions && !isLikelyTransactionMainRowText(txt) && !isTransactionsSectionHeader(txt) && !isDebitCreditHeaderLine(txt) && isHeaderNoiseLine(txt)) {
            out.debugLines.push_back(std::string("line.skip.headerNoise\t") + txt + "\tline=" + std::to_string(li));
            prevLine = txt;
            continue;
        }

        if (!inTransactions) {
            const bool canStart = (!currentBookingDate.empty()) || (rows.inSection && cols.hasValuta());
            if (canStart && (isLikelyTransactionMainRowText(txt) || isLikelyTransactionMainRowGeom(l, cols))) {
                inTransactions = true;
                out.debugLines.push_back(std::string("tx.start\tline=") + std::to_string(li) + "\ttext=" + txt);
            } else {
                out.debugLines.push_back(std::string("line.skip.notInTransactions\t") + txt + "\tline=" + std::to_string(li));
                prevLine = txt;
                continue;
            }
        }

        const bool mainByRegex = isLikelyTransactionMainRowText(txt);
        const bool mainByGeom = (!mainByRegex) && isLikelyTransactionMainRowGeom(l, cols);

        if (mainByRegex || mainByGeom) {
            if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
            cur.bookingDateGroup = currentBookingDate;
            cur.main = splitMainRow(l, cols);

            // NOTE: Do not mutate cur.main here. Table-cell mapping is performed later when finalizing blocks.

            if (mainByGeom) {
                out.debugLines.push_back(std::string("tx.main.geom\t") + txt + "\tline=" + std::to_string(li));
            }

            {
                const auto toks = utils::splitWhitespace(l.text);
                if (toks.size() == l.wordSpans.size()) {
                    auto idxNearX = [&](int x) -> int {
                        int bestIdx = -1; int bestDist = std::numeric_limits<int>::max();
                        for (size_t i = 0; i < toks.size(); ++i) {
                            const auto& sp = l.wordSpans[i];
                            const int cx = (sp.first + sp.second) / 2;
                            const int d = std::abs(cx - x);
                            if (d < bestDist) { bestDist = d; bestIdx = static_cast<int>(i); }
                        }
                        return bestIdx;
                    };
                    int valutaIdx = cols.hasValuta() ? idxNearX(cols.valutaX) : -1;
                    int debitIdx = cols.hasDebit() ? idxNearX(cols.debitX) : -1;
                    int creditIdx = cols.hasCredit() ? idxNearX(cols.creditX) : -1;

                    std::ostringstream sel;
                    sel << "tx.main.split\tline=" << li << "\tvalutaIdx=" << valutaIdx << "\tdebitIdx=" << debitIdx << "\tcreditIdx=" << creditIdx;
                    out.debugLines.push_back(sel.str());

                    std::ostringstream toksOut; toksOut << "tx.main.tokens\tline=" << li;
                    for (size_t ti = 0; ti < toks.size(); ++ti) {
                        const auto& sp = l.wordSpans[ti];
                        const int cx = (sp.first + sp.second) / 2;
                        toksOut << " \"" << toks[ti] << "\"@" << cx;
                    }
                    out.debugLines.push_back(toksOut.str());
                }
            }

            out.debugLines.push_back(std::string("tx.main\t") + txt + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("tx.main.left\t") + cur.main.left.line.text);
            if (!cur.main.valuta.empty()) out.debugLines.push_back(std::string("tx.main.valuta\t") + cur.main.valuta.line.text);
            if (!cur.main.debit.empty()) out.debugLines.push_back(std::string("tx.main.debit\t") + cur.main.debit.line.text);
            if (!cur.main.credit.empty()) out.debugLines.push_back(std::string("tx.main.credit\t") + cur.main.credit.line.text);
            prevLine = txt;
            continue;
        }

        if (!cur.main.left.empty()) {
            if (cols.hasValuta()) {
                const auto toks = utils::splitWhitespace(l.text);
                if (toks.size() == l.wordSpans.size()) {
                    size_t cut = toks.size();
                    for (size_t i = 0; i < toks.size(); ++i) {
                        const auto& sp = l.wordSpans[i];
                        const int cx = (sp.first + sp.second) / 2;
                        if (cx >= cols.valutaX) { cut = i; break; }
                    }
                    OcrLine dl = toOcrLineFromWords(l, 0, cut);
                    if (!trim(dl.text).empty()) {
                        cur.detailLines.push_back(std::move(dl));
                        out.debugLines.push_back(std::string("detail.append\tline=") + std::to_string(li) + "\ttext=" + l.text + "\tcut=" + std::to_string(cut));
                    }
                } else {
                    OcrLine dl;
                    dl.minX = l.minX; dl.maxX = l.maxX; dl.minY = l.minY; dl.maxY = l.maxY; dl.wordSpans = l.wordSpans;
                    dl.text = l.text;
                    cur.detailLines.push_back(std::move(dl));
                    out.debugLines.push_back(std::string("detail.append.unknown\tline=") + std::to_string(li) + "\ttext=" + l.text);
                }
            } else {
                OcrLine dl;
                dl.minX = l.minX; dl.maxX = l.maxX; dl.minY = l.minY; dl.maxY = l.maxY; dl.wordSpans = l.wordSpans;
                dl.text = l.text;
                cur.detailLines.push_back(std::move(dl));
                out.debugLines.push_back(std::string("detail.append.novaluta\tline=") + std::to_string(li) + "\ttext=" + l.text);
            }
        }

        prevLine = txt;
    }

    flush();

    out.debugLines.push_back(std::string("blocks\t") + std::to_string(blocks.size()));

    for (const auto& b : blocks) {
        if (b.main.left.empty()) continue;

        // Diagnostic debug: log block main cell texts and column model state
        try {
            out.debugLines.push_back(std::string("block.debug\tbookingDateGroup=") + b.bookingDateGroup);
            out.debugLines.push_back(std::string("block.main.left.text\t") + b.main.left.line.text);
            out.debugLines.push_back(std::string("block.main.valuta.empty\t") + (b.main.valuta.empty() ? "1" : "0"));
            out.debugLines.push_back(std::string("block.main.valuta.text\t") + (b.main.valuta.empty() ? std::string("(none)") : b.main.valuta.line.text));
            out.debugLines.push_back(std::string("block.main.debit.empty\t") + (b.main.debit.empty() ? "1" : "0"));
            out.debugLines.push_back(std::string("block.main.debit.text\t") + (b.main.debit.empty() ? std::string("(none)") : b.main.debit.line.text));
            out.debugLines.push_back(std::string("block.main.credit.empty\t") + (b.main.credit.empty() ? "1" : "0"));
            out.debugLines.push_back(std::string("block.main.credit.text\t") + (b.main.credit.empty() ? std::string("(none)") : b.main.credit.line.text));
            out.debugLines.push_back(std::string("cols.state\tvalutaX=") + std::to_string(cols.valutaX) + std::string("\tdebitX=") + std::to_string(cols.debitX) + std::string("\tcreditX=") + std::to_string(cols.creditX));
        } catch (...) {}

        const auto txp = DefaultTransactionParser::parseTransaction(b);

        Transaction tx;
        tx.name = "Transaction " + std::to_string(txIndex++);
        tx.bookingDate = txp.bookingDate;
        tx.valuta = txp.valuta;
        tx.amount = txp.amount;
        tx.description = txp.description;
        tx.actorProposal = txp.actorProposal;
        tx.metadata = txp.metadata;

        // If OCR table cells exist, prefer parsing amounts from the corresponding cell text
         try {
             if (!ocr.tables.empty() && ocr.tables[0].cells.size() > 0) {
                const auto& t = ocr.tables[0];
                auto overlapsLine = [&](const api::tesseract::Cell& c)->bool{
                    const int cTop = c.bbox.y;
                    const int cBot = c.bbox.y + c.bbox.height;
                    return !(cTop > b.main.left.line.maxY || cBot < b.main.left.line.minY);
                };

                bool usedCell = false;
                // Prefer exact column matches if we inferred table columns
                if (cols.hasCredit() && cols.creditCol >= 0) {
                    for (const auto& c : t.cells) {
                        if (c.col == cols.creditCol && overlapsLine(c)) {
                            out.debugLines.push_back(std::string("cell.amount_used\t") + c.text);
                            if (auto v = core::parser::parseAmountString(c.text)) { tx.amount = *v; usedCell = true; }
                            break;
                        }
                    }
                }
                if (!usedCell && cols.hasDebit() && cols.debitCol >= 0) {
                    for (const auto& c : t.cells) {
                        if (c.col == cols.debitCol && overlapsLine(c)) {
                            out.debugLines.push_back(std::string("cell.amount_used\t") + c.text);
                            if (auto v = core::parser::parseAmountString(c.text)) { tx.amount = -std::abs(*v); usedCell = true; }
                            break;
                        }
                    }
                }
             }
         } catch (...) {}

        // Per-transaction proof crop (restore original behavior)
        try {
            if (opencv && !pageCropImagePath.empty()) {
                int minX = std::numeric_limits<int>::max();
                int maxX = std::numeric_limits<int>::min();
                int minY = std::numeric_limits<int>::max();
                int maxY = std::numeric_limits<int>::min();

                auto accBounds = [&](const OcrLine& l) {
                    minX = std::min(minX, l.minX);
                    maxX = std::max(maxX, l.maxX);
                    minY = std::min(minY, l.minY);
                    maxY = std::max(maxY, l.maxY);
                };

                if (!b.main.left.empty()) accBounds(b.main.left.line);
                for (const auto& l : b.detailLines) accBounds(l);

                if (minX != std::numeric_limits<int>::max()) {
                    api::opencv::CropRequest creq;
                    creq.imagePath = std::filesystem::path(pageCropImagePath);
                    creq.outputDir = std::filesystem::path(pageCropImagePath).parent_path();
                    creq.uniqIdPrefix = std::string(utils::makeUniqId());
                    creq.filePrefix = std::string("opencv_proof_tx") + std::to_string(txIndex);
                    creq.outputFormat = api::opencv::CropRequest::OutputFormat::Jpg;
                    creq.jpegQuality = 92;

                    // keep original behavior: use full width so proof crop contains valuta/debit/credit columns
                    creq.bbox.x = 0;
                    creq.bbox.y = std::max(0, minY - 20);
                    creq.bbox.width = 1 << 30;

                    creq.bbox.height = std::max(1, (maxY - minY) + 24);
                    try {
                        out.debugLines.push_back(std::string("crop.request\t") + creq.imagePath.string() + std::string("\tfilePrefix=") + creq.filePrefix);
                        const auto cres = opencv->crop(creq);
                        out.debugLines.push_back(std::string("crop.result.count\t") + std::to_string(cres.croppedImagePaths.size()));
                        if (!cres.croppedImagePaths.empty()) {
                            try { tx.proofImagePath = std::filesystem::absolute(cres.croppedImagePaths.front()).string(); }
                            catch (...) { tx.proofImagePath = cres.croppedImagePaths.front().string(); }
                        }
                    } catch (const std::exception& ex) {
                        out.debugLines.push_back(std::string("crop.exception\t") + ex.what());
                    } catch (...) {
                        out.debugLines.push_back(std::string("crop.exception\tunknown"));
                    }
                }
            }
        } catch (...) {}

         out.transactions.push_back(std::make_shared<Transaction>(std::move(tx)));
    }

    out.debugLines.push_back(std::string("transactions	") + std::to_string(out.transactions.size()));

    out.lastBookingDate = currentBookingDate;
    out.nextTransactionIndex = txIndex;
    return out;
}
