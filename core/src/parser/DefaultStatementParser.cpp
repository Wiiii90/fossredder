#include "core/pch.h"

#include "core/parser/DefaultStatementParser.h"

#include "core/parser/DefaultTransactionParser.h"
#include "core/parser/ParserHeuristics.h"
#include "core/parser/ParserHelpers.h"
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
using core::parser::OcrLine;
using core::parser::TransactionBlock;
using utils::lowerAscii;
using utils::trim;

namespace {

static bool isValutaHeaderLine(const std::string& line) {
    const auto n = core::parser::helpers::normalizeAlnumLower(line);
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

// Forward declarations to ensure visibility before use
static core::parser::OcrLine rawToOcrLine(const RawLine& l);
static bool isLikelyTransactionMainRowGeom(const RawLine& l, const ColumnModel& cols);

// Convert internal RawLine to public OcrLine structure
static core::parser::OcrLine rawToOcrLine(const RawLine& l) {
    core::parser::OcrLine ol;
    ol.minX = l.minX; ol.maxX = l.maxX; ol.minY = l.minY; ol.maxY = l.maxY; ol.wordSpans = l.wordSpans; ol.text = l.text;
    return ol;
}

// Geometry-based heuristic: detect whether a raw line looks like a transaction main row
static bool isLikelyTransactionMainRowGeom(const RawLine& l, const ColumnModel& cols) {
    if (!cols.hasValuta() || (!cols.hasDebit() && !cols.hasCredit())) return false;
    const auto toks = utils::splitWhitespace(l.text);
    if (toks.size() != l.wordSpans.size() || toks.size() < 3) return false;

    const int band = core::parser::helpers::parserConfig.tokenNearBandForMainRow;
    const bool hasValuta = core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.valutaX, band);
    bool hasDebit = cols.hasDebit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.debitX, band) : false;
    bool hasCredit = cols.hasCredit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.creditX, band) : false;

    // If neither debit nor credit detected, accept numeric-like token near the valuta column
    if (!hasDebit && !hasCredit) {
        try {
            auto amtIdx = core::parser::helpers::findAmountTokenIndices(rawToOcrLine(l), cols.valutaX, core::parser::helpers::parserConfig.amountNearValutaBandPx);
            if (!amtIdx.empty()) hasDebit = true;
        } catch(...) {}
    }

    bool hasLeft = false;
    for (size_t i = 0; i < toks.size(); ++i) {
        const auto& sp = l.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (cx < cols.valutaX - core::parser::helpers::parserConfig.leftDescriptiveOffsetPx) { hasLeft = true; break; }
    }

    return hasLeft && hasValuta && (hasDebit || hasCredit);
}

using core::parser::heuristics::isFooterLine;
using core::parser::heuristics::isHeaderNoiseLine;
using core::parser::heuristics::isPostTransactionFootnote;
using core::parser::heuristics::isTransactionsSectionHeader;
using core::parser::heuristics::isDebitCreditHeaderLine;

static bool isLikelyTransactionMainRowText(const std::string& line) {
    // Allow optional whitespace between day and month produced by OCR (e.g. "01. 04")
    static const std::regex re(R"((\d{2}\.\s*\d{2})\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?(?:\s+.*)?\s*$)");
    return std::regex_search(line, re);
}

// Returns true if the line is likely a transaction header line
static bool isLikelyTransactionHeaderLine(const OcrLine& l, const ColumnModel& cols) {
    return (cols.hasValuta() && core::parser::helpers::hasTokenNearX(l, cols.valutaX, 100));
}

} // anonymous namespace

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

    // Build OcrLines and infer columns using centralized helpers
    const auto ocrLines = core::parser::helpers::buildOcrLinesFromWords(ocr.words);
    out.debugLines.push_back(std::string("ocr.words\t") + std::to_string(ocr.words.size()));
    out.debugLines.push_back(std::string("ocr.lines.raw\t") + std::to_string(ocrLines.size()));

    const auto seed = core::parser::helpers::inferColumnModelFromLines(ocrLines);
    ColumnModel seedCols{ seed.valutaX, seed.debitX, seed.creditX, -1, -1, -1 };

    // Convert OcrLines to RawLineLite for selective merging
    std::vector<core::parser::helpers::detail::RawLineLite> rawLite;
    rawLite.reserve(ocrLines.size());
    for (const auto& l : ocrLines) rawLite.push_back({ l.minX, l.maxX, l.minY, l.maxY, l.wordSpans, l.text });

    const auto merged = core::parser::helpers::selectiveGroupMergeLinesRaw(rawLite, 8, { seed.valutaX, seed.debitX, seed.creditX });
    // convert merged back to RawLine
    std::vector<RawLine> lines; lines.reserve(merged.size());
    for (const auto& ml : merged) lines.push_back(RawLine{0, ml.minX, ml.maxX, ml.minY, ml.maxY, ml.wordSpans, ml.text});
    out.debugLines.push_back(std::string("ocr.lines\t") + std::to_string(lines.size()));

    // OcrLine view for merged raw lines (used by helpers)
    std::vector<OcrLine> ocrFromRaw; ocrFromRaw.reserve(lines.size());
    for (const auto& rl : lines) ocrFromRaw.push_back(rawToOcrLine(rl));

    std::string currentBookingDate = std::move(initialBookingDate);
    int txIndex = std::max(1, initialTransactionIndex);

    // Early empty-page detection: centralized helper
    {
        std::string dbg;
        std::string foundDate;
        if (core::parser::helpers::detectEarlyEmptyPage(ocrLines, dbg, foundDate)) {
            out.debugLines.push_back(dbg);
            out.debugLines.push_back(std::string("page.emptyDetected\tlines=") + std::to_string(12));
            out.lastBookingDate = currentBookingDate;
            out.nextTransactionIndex = txIndex;
            return out;
        }
        if (!dbg.empty()) out.debugLines.push_back(dbg);
    }

    std::vector<TransactionBlock> blocks;
    TransactionBlock cur;
    cur.bookingDateGroup = currentBookingDate;

    bool inTransactions = !currentBookingDate.empty();
    ColumnModel cols = seedCols;
    RowModel rows;

    // Pre-scan lines to determine a conservative header bottom Y so we do not start parsing
    // transactions from text that is clearly part of the page header. This prevents lookahead
    // from combining header lines into a transaction before header detection runs in the loop.
    const size_t headerScanLines = 12; // reduced window for header detection
    auto [preHeaderBottomY, headerFound] = core::parser::helpers::detectHeaderRegion(ocrLines, headerScanLines, seed.valutaX);
    out.debugLines.push_back(std::string("header.prebottomY\t") + std::to_string(preHeaderBottomY));
    out.debugLines.push_back(std::string("header.found\t") + (headerFound ? "1" : "0"));
    // Diagnostic: enumerate first scan window lines and log header-like signals to aid debugging
    try {
        const size_t dbgN = std::min(ocrLines.size(), headerScanLines);
        for (size_t hi = 0; hi < dbgN; ++hi) {
            const auto& hl = ocrLines[hi];
            const auto text = hl.text;
            try {
                bool isTxSection = core::parser::heuristics::isTransactionsSectionHeader(text);
                bool isDebitCredit = core::parser::heuristics::isDebitCreditHeaderLine(text);
                bool isNoise = core::parser::heuristics::isHeaderNoiseLine(text);
                bool hasFullDate = core::parser::helpers::findFirstFullDate(text).has_value();
                core::parser::OcrLine ol = hl;
                bool hasAmt = core::parser::helpers::hasAmountLikeTokenInLine(ol, seed.valutaX);
                bool hasValutaTokenNear = false;
                try { if (seed.valutaX >= 0) hasValutaTokenNear = core::parser::helpers::hasTokenNearX(ol, seed.valutaX, core::parser::helpers::parserConfig.tokenNearBandForMainRow); } catch(...) {}
                std::ostringstream ss; ss << "header.candidate\tline=" << hi << "\ttext=" << text << "\ttxSection=" << (isTxSection?"1":"0") << "\tdebitcredit=" << (isDebitCredit?"1":"0") << "\tnoise=" << (isNoise?"1":"0") << "\tfullDate=" << (hasFullDate?"1":"0") << "\thasAmt=" << (hasAmt?"1":"0") << "\tvalutaNear=" << (hasValutaTokenNear?"1":"0");
                out.debugLines.push_back(ss.str());
            } catch(...) {}
        }
    } catch(...) {}

    int headerBottomY = preHeaderBottomY; // track lowest (largest y) header line encountered
    // If we find an early line that already contains an amount-like token or short-date+left-text,
    // lower the headerBottomY so we do not skip legitimate transaction starts.
    try {
        int earliestAmtY = std::numeric_limits<int>::max();
        bool found = false;
        for (size_t i = 0; i < lines.size(); ++i) {
            try {
                core::parser::OcrLine ol = rawToOcrLine(lines[i]);
                bool hasAmt = core::parser::helpers::hasAmountLikeTokenInLine(ol, seed.valutaX);
                bool dateLeft = core::parser::helpers::hasShortDateToken(ol.text) && core::parser::helpers::hasLeftDescriptiveText(ol, seed.valutaX);
                if (hasAmt || dateLeft) {
                    earliestAmtY = std::min(earliestAmtY, lines[i].maxY);
                    found = true;
                }
            } catch(...) {}
        }
        if (found && earliestAmtY != std::numeric_limits<int>::max()) {
            // reduce header bottom to just above earliest amount line to avoid skipping it
            headerBottomY = std::min(headerBottomY, earliestAmtY - 2);
            out.debugLines.push_back(std::string("header.adjustedForEarliestAmount\t") + std::to_string(headerBottomY));
        }
    } catch(...) {}

    const int headerMarginPx = 8; // smaller margin to avoid skipping legitimate lines

    // Compute page maximum Y for heuristics (used to decide if a footnote is truly at page bottom)
    int pageMaxY = -1;
    try {
        for (const auto& l : lines) pageMaxY = std::max(pageMaxY, l.maxY);
    } catch(...) { pageMaxY = -1; }

    // Booking-date fallback: try centralized helper
    if (currentBookingDate.empty()) {
        if (auto d = core::parser::helpers::findFallbackBookingDate(ocrLines, 30)) {
            currentBookingDate = *d;
            out.debugLines.push_back(std::string("header.fallbackBookingDate\t") + currentBookingDate);
            cur.bookingDateGroup = currentBookingDate;
        }
    }

    const auto flush = [&]() {
         if (cur.main.left.empty() && cur.detailLines.empty()) return;
         blocks.push_back(std::move(cur));
         cur = TransactionBlock{};
         cur.bookingDateGroup = currentBookingDate;
     };

    std::string prevLine;
    int txStartLooseCount = 0;

    std::vector<OcrLine> orphanLines; // lines skipped before any main row found

    for (size_t li = 0; li < lines.size(); ++li) {
        const auto& l = lines[li];
        const auto txt = l.text;
        const auto combined = prevLine.empty() ? txt : (prevLine + " " + txt);
        if (txt.empty()) continue;

        if (isFooterLine(txt)) { out.debugLines.push_back(std::string("stop.footer\t") + txt + "\tline=" + std::to_string(li)); break; }
        // Only treat post-transaction footnote as terminal if we already started parsing transactions
        // or have found blocks, or the footnote lies near the bottom of the page. This avoids
        // prematurely stopping when header-like lines appear near the header bottom.
        if (isPostTransactionFootnote(txt)) {
            bool nearBottom = false;
            try { if (pageMaxY >= 0 && l.maxY >= static_cast<int>(pageMaxY * 3 / 4)) nearBottom = true; } catch(...) {}
            // Only treat as terminal when we already have found transaction blocks and
            // the footnote is clearly near the bottom of the page. This avoids
            // aborting pages where an initial booking date is provided but no
            // transactions have yet been detected on this page.
            try {
                out.debugLines.push_back(std::string("footnote.check\tpageMaxY=") + std::to_string(pageMaxY) + std::string("\tlineY=") + std::to_string(l.maxY) + std::string("\tnearBottom=") + (nearBottom ? "1" : "0") + std::string("\tblocks=") + std::to_string(blocks.size()));
            } catch(...) {}

            bool reallyTerminal = false;
            try { if (!blocks.empty() && nearBottom) reallyTerminal = true; } catch(...) {}
            if (reallyTerminal) {
                out.debugLines.push_back(std::string("stop.footnote\t") + txt + "\tline=" + std::to_string(li));
                break;
            }
            out.debugLines.push_back(std::string("footnote.ignored\t") + txt + "\tline=" + std::to_string(li));
            prevLine = txt;
            continue; // skip footnote content lines entirely
        }

        // If we've precomputed a header bottom, skip any lines that lie above it
        // (avoid parsing page header). Do this regardless of inTransactions so that
        // header/footer text near the top of the page is not misinterpreted as
        // transactions even when a booking date was provided.
        if (headerBottomY >= 0 && l.maxY <= headerBottomY + headerMarginPx && !inTransactions) {
            // If this line contains tokens anchored to the inferred valuta column, treat it as data (do not skip).
            try {
                if (seedCols.valutaX >= 0) {
                    core::parser::OcrLine anchorOl = rawToOcrLine(l);
                    if (core::parser::helpers::hasTokenNearX(anchorOl, seedCols.valutaX, core::parser::helpers::parserConfig.tokenNearBandForMainRow) || core::parser::helpers::hasAmountLikeTokenInLine(anchorOl, seedCols.valutaX)) {
                        out.debugLines.push_back(std::string("header.overrideByValutaAnchor\tline=") + std::to_string(li) + "\ttext=" + txt);
                        // fall through to normal processing (do not perform header skip)
                    } else {
                        // proceed with header-skip checks below
                        continue;
                    }
                }
            } catch(...) {}

            // Do not skip lines that contain explicit header signals such as a booking date,
            // 'valuta' or any full date token. This prevents legitimate header/data lines
            // from being swallowed by conservative header bounding.
            bool hasHeaderSignal = false;
            try {
                if (core::parser::helpers::findBookingDateInHeader(txt).has_value()) hasHeaderSignal = true;
            } catch(...) {}
            try {
                // also consider the combined previous+current token for split OCR cases
                if (core::parser::helpers::findBookingDateInHeader(combined).has_value()) hasHeaderSignal = true;
            } catch(...) {}
            try {
                if (isValutaHeaderLine(txt)) hasHeaderSignal = true;
            } catch(...) {}

            if (!hasHeaderSignal && !(rows.inSection && cols.hasValuta())) {
                // before skipping, attempt combined prev+curr / curr+next main-row detection
                bool combinedStarted = false;
                try {
                    std::string combPrev = prevLine.empty() ? txt : (prevLine + std::string(" ") + txt);
                    std::string combNext = txt;
                    if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li+1].text;
                    try {
                        if (auto cs = core::parser::helpers::tryCombinedStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true;
                            out.debugLines.push_back(std::string("tx.start.combined.aboveHeader\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                            combinedStarted = true;
                        }
                    } catch(...) {}
                } catch(...) {}
                if (combinedStarted) {
                    // allow processing as normal
                } else {
                    // If the line contains an amount-like token or matches a loose transaction pattern,
                    // allow it to start transactions even though it's above the conservative header bound.
                    try {
                        core::parser::OcrLine ol = rawToOcrLine(l);
                        bool allowAboveHeader = false;
                        try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) allowAboveHeader = true; } catch(...) {}
                        try { if (!allowAboveHeader && core::parser::helpers::isLooseTransactionLine(ol, cols.valutaX)) allowAboveHeader = true; } catch(...) {}
                        if (allowAboveHeader) {
                            inTransactions = true;
                            ++txStartLooseCount;
                            out.debugLines.push_back(std::string("tx.start.aboveHeaderDetected\tline=") + std::to_string(li) + "\ttext=" + txt);
                        } else {
                            out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                            prevLine = txt;
                            continue;
                        }
                    } catch(...) {
                        out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                        prevLine = txt;
                        continue;
                    }
                }
            }
            // otherwise fall through and allow processing of this line
        }

        if ((!cols.hasDebit() || !cols.hasCredit()) && isDebitCreditHeaderLine(txt)) {
            if (!cols.hasDebit()) {
                if (auto dx = core::parser::helpers::findPhraseCenterX(rawToOcrLine(l), {"zu","ihren","lasten"})) cols.debitX = *dx;
            }
            if (!cols.hasCredit()) {
                if (auto cx = core::parser::helpers::findPhraseCenterX(rawToOcrLine(l), {"zu","ihren","gunsten"})) cols.creditX = *cx;
            }
            out.debugLines.push_back(std::string("header.debitcredit\t") + txt + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.debitX\t") + std::to_string(cols.debitX) + "\tcols.creditX\t" + std::to_string(cols.creditX));
            // update header bottom
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch(...) {}
            prevLine = txt;
            continue;
        }

        if (!rows.inSection && (isTransactionsSectionHeader(txt) || isValutaHeaderLine(txt) || isTransactionsSectionHeader(combined))) {
            rows.inSection = true;
            if (!cols.hasValuta()) {
                if (auto vx = core::parser::helpers::findTokenCenterX(rawToOcrLine(l), "valuta")) cols.valutaX = *vx;
            }
            out.debugLines.push_back(std::string("header.section\t") + combined + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.valutaX\t") + std::to_string(cols.valutaX));
            // update header bottom
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch(...) {}
            prevLine = txt;
            continue;
        }

        if (auto bd = core::parser::helpers::findBookingDateInHeader(txt)) {
            currentBookingDate = *bd;
            out.debugLines.push_back(std::string("header.bookingDate\t") + currentBookingDate + "\tline=" + std::to_string(li));
            flush();
            cur.bookingDateGroup = currentBookingDate;
            // update header bottom
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch(...) {}
            prevLine = txt;
            continue;
        }

        // Header-noise filtering: skip obvious header-noise lines only when we detected a header region
        // and the line is not anchored to a valuta/amount/date/main-row signal. This avoids skipping
        // legitimate transaction-like lines while removing repeated header boilerplate.
        bool headerNoiseSkipped = false;
        try {
            if (!inTransactions && headerFound && core::parser::heuristics::isHeaderNoiseLine(txt)) {
                bool anchoredToValuta = false;
                try { if (seedCols.valutaX >= 0) { core::parser::OcrLine _ol = rawToOcrLine(l); if (core::parser::helpers::hasTokenNearX(_ol, seedCols.valutaX, core::parser::helpers::parserConfig.tokenNearBandForMainRow) || core::parser::helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) anchoredToValuta = true; } } catch(...) {}

                bool looksMain = false;
                try {
                    core::parser::OcrLine _ol = rawToOcrLine(l);
                    if (core::parser::helpers::isLooseTransactionLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (core::parser::helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (core::parser::helpers::hasShortDateToken(_ol.text) && core::parser::helpers::hasLeftDescriptiveText(_ol, seedCols.valutaX)) looksMain = true;
                } catch(...) {}

                if (!anchoredToValuta && !looksMain && !isTransactionsSectionHeader(txt) && !isDebitCreditHeaderLine(txt) && !isLikelyTransactionMainRowText(txt)) {
                    out.debugLines.push_back(std::string("line.skip.headerNoise\t") + txt + std::string("\tline=") + std::to_string(li));
                    prevLine = txt;
                    headerNoiseSkipped = true;
                } else {
                    out.debugLines.push_back(std::string("header.noise_filter.suppressed\tline=") + std::to_string(li));
                }
            }
        } catch(...) {}
        if (headerNoiseSkipped) continue;

        if (!inTransactions) {
            const bool canStart = (!currentBookingDate.empty()) || (rows.inSection && cols.hasValuta());
            bool didVerticalStart = false;
            // conservative vertical merge: check neighbor lines for amount/date relationship
            try {
                if (canStart && rows.inSection && cols.hasValuta()) {
                    if (auto vs = core::parser::helpers::tryVerticalStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX }, currentBookingDate)) {
                        inTransactions = true; didVerticalStart = true;
                        if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
                        cur.bookingDateGroup = currentBookingDate;
                        cur.main = vs->first;
                        if (vs->second > 0) {
                            out.debugLines.push_back(std::string("tx.main.vertical.helper\tformed\tline=") + std::to_string(li));
                            ++li;
                        } else {
                            out.debugLines.push_back(std::string("tx.main.vertical.helper\tformed\tline=") + std::to_string(li));
                        }
                    }
                }
            } catch(...) {}

            if (canStart && (isLikelyTransactionMainRowText(txt) || isLikelyTransactionMainRowGeom(l, cols))) {
                 inTransactions = true;
                 out.debugLines.push_back(std::string("tx.start\tline=") + std::to_string(li) + "\ttext=" + txt);
            } else if (didVerticalStart) {
                // vertical start happened; continue processing the line as inTransactions
            } else {
                // try combined prev+curr and curr+next matching for regex based starts
                bool startedCombined = false;
                try {
                    std::string combPrev = prevLine.empty() ? txt : (prevLine + std::string(" ") + txt);
                    std::string combNext = txt;
                    if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li+1].text;
                    try {
                        if (auto cs = core::parser::helpers::tryCombinedStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true; startedCombined = true;
                            out.debugLines.push_back(std::string("tx.start.combined\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                        }
                    } catch(...) {}
                } catch(...) {}
                if (startedCombined) {
                    // continue and let normal main detection handle the line
                } else {
                    // Additional relaxed start heuristic: if the line contains a short date token and an amount-like token
                    // (or descriptive left text), treat it as a transaction start. Also check neighboring lines for amount tokens.
                    bool relaxedStart = false;
                    try {
                        core::parser::OcrLine ol = rawToOcrLine(l);
                        bool hasDate = false;
                        try { if (core::parser::helpers::hasShortDateToken(ol.text)) hasDate = true; } catch(...) {}
                        bool hasAmount = false;
                        try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) hasAmount = true; } catch(...) {}
                        // check previous and next lines for amount tokens as fallback
                        try {
                            if (!hasAmount && li > 0) { core::parser::OcrLine prevOl = rawToOcrLine(lines[li-1]); if (core::parser::helpers::hasAmountLikeTokenInLine(prevOl, cols.valutaX)) hasAmount = true; }
                            if (!hasAmount && li + 1 < lines.size()) { core::parser::OcrLine nextOl = rawToOcrLine(lines[li+1]); if (core::parser::helpers::hasAmountLikeTokenInLine(nextOl, cols.valutaX)) hasAmount = true; }
                        } catch(...) {}
                        bool hasLeftDesc = false;
                        try { if (core::parser::helpers::hasLeftDescriptiveText(ol, cols.valutaX)) hasLeftDesc = true; } catch(...) {}
                        if (hasDate && (hasAmount || hasLeftDesc)) relaxedStart = true;
                    } catch(...) {}
                    if (relaxedStart) {
                        inTransactions = true;
                        out.debugLines.push_back(std::string("tx.start.relaxed\tline=") + std::to_string(li) + "\ttext=" + txt);
                    } else {
                    // Previously we skipped this line as header/noise. Disable skipping: force-start a transaction
                    inTransactions = true;
                    out.debugLines.push_back(std::string("tx.start.forced\tline=") + std::to_string(li) + "\ttext=" + txt);
                    if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
                    cur.bookingDateGroup = currentBookingDate;
                    core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
                    cur.main = core::parser::helpers::handleMainRow(rawToOcrLine(l), cg, false, out.debugLines);
                    prevLine = txt;
                    continue;
                    }
                }
            }
        }

        const bool mainByRegex = isLikelyTransactionMainRowText(txt);
        // Allow geometry-based main-row detection earlier when we have an inferred valuta column
        ColumnModel effectiveCols = cols.hasValuta() ? cols : seedCols;
        const bool mainByGeom = (!mainByRegex) && ((li > headerScanLines) || effectiveCols.hasValuta()) && isLikelyTransactionMainRowGeom(l, effectiveCols);

        if (mainByRegex || mainByGeom) {
            if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
            cur.bookingDateGroup = currentBookingDate;
            core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
            cur.main = core::parser::helpers::handleMainRow(rawToOcrLine(l), cg, mainByGeom, out.debugLines);
            prevLine = txt;
            continue;
        }

        if (!cur.main.left.empty()) {
            core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
            core::parser::helpers::appendDetailLine(cur, rawToOcrLine(l), cg, &out.debugLines);
        }

        prevLine = txt;
    }

    flush();

    // Post-processing: attach orphan lines via centralized helper
    // Rescue orphan lines that look like main transaction rows (date + amount or left descriptive + date)
    try {
        for (const auto& ol : orphanLines) {
            try {
                bool looksMain = false;
                try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) looksMain = true; } catch(...) {}
                try { if (!looksMain && core::parser::helpers::isLooseTransactionLine(ol, cols.valutaX)) looksMain = true; } catch(...) {}
                try { if (!looksMain && core::parser::helpers::hasShortDateToken(ol.text) && core::parser::helpers::hasLeftDescriptiveText(ol, cols.valutaX)) looksMain = true; } catch(...) {}
                if (looksMain) {
                    TransactionBlock nb;
                    nb.bookingDateGroup = currentBookingDate;
                    core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
                    nb.main = core::parser::helpers::handleMainRow(ol, cg, false, out.debugLines);
                    blocks.push_back(std::move(nb));
                    out.debugLines.push_back(std::string("tx.start.rescued\ttext=") + ol.text);
                }
            } catch(...) {}
        }
    } catch(...) {}

    core::parser::helpers::attachOrphansToBlocks(blocks, orphanLines, 80, cols.valutaX);

    out.debugLines.push_back(std::string("blocks\t") + std::to_string(blocks.size()));

    // concise page summary
    try {
        std::ostringstream ps;
        ps << "page.summary\tocr.lines=" << lines.size() << "\theader.prebottomY=" << headerBottomY << "\tblocks=" << blocks.size() << "\ttxStartLoose=" << txStartLooseCount;
        out.debugLines.push_back(ps.str());
    } catch(...) {}

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

        // collect per-transaction debug lines
        std::vector<std::string> txDebug;
        const auto txp = DefaultTransactionParser::parseTransaction(b, &txDebug);
        for (const auto &d : txDebug) out.debugLines.push_back(std::string("txdbg\t") + d);

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
                            if (auto v = core::parser::parseAmountString(c.text)) { tx.amount = *v; usedCell = true; out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount)); }
                            break;
                        }
                    }
                }
                if (!usedCell && cols.hasDebit() && cols.debitCol >= 0) {
                    for (const auto& c : t.cells) {
                        if (c.col == cols.debitCol && overlapsLine(c)) {
                            out.debugLines.push_back(std::string("cell.amount_used\t") + c.text);
                            if (auto v = core::parser::parseAmountString(c.text)) { tx.amount = -std::abs(*v); usedCell = true; out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount)); }
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

                    // keep original behavior: use full width so proof crop contains valida/debit/credit columns
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

    out.debugLines.push_back(std::string("transactions\t") + std::to_string(out.transactions.size()));

    out.lastBookingDate = currentBookingDate;
    out.nextTransactionIndex = txIndex;
    return out;
}
