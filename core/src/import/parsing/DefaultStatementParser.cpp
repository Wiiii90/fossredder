#include "core/pch.h"

#include "DefaultStatementParser.h"

#include "core/constants/CoreDefaults.h"

#include "DefaultTransactionParser.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "ParserHeuristics.h"
#include "ParserHelpers.h"
#include "StatementParseHelpers.h"
#include "../../utils/UniqId.h"
#include "../../utils/Util.h"

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
using core::parser::detail::ColumnModel;
using core::parser::detail::HeaderAnalysis;
using core::parser::detail::RawLine;
using core::parser::detail::analyzeHeaderWindow;
using core::parser::detail::appendDetailLine;
using core::parser::detail::appendPageSummary;
using core::parser::detail::appendTransactionsFromBlocks;
using core::parser::detail::attachOrphansToBlocks;
using core::parser::detail::detectEarlyEmptyPage;
using core::parser::detail::findBookingDateInHeader;
using core::parser::detail::findFallbackBookingDate;
using core::parser::detail::handleMainRow;
using core::parser::detail::isLikelyTransactionHeaderLine;
using core::parser::detail::isLikelyTransactionMainRowGeom;
using core::parser::detail::rawToOcrLine;
using core::parser::detail::rescueOrphanBlocks;
using core::parser::detail::selectiveGroupMergeLinesRaw;
using core::parser::detail::tryCombinedStart;
using core::parser::detail::tryVerticalStart;
using utils::lowerAscii;
using utils::trim;

namespace {

static bool isValutaHeaderLine(const std::string& line) {
    const auto n = core::parser::helpers::normalizeAlnumLower(line);
    return n.find("valuta") != std::string::npos;
}

struct RowModel {
    bool inSection = false;
};

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

} // anonymous namespace

DefaultStatementParser::ParseResult DefaultStatementParser::parse(const api::opencv::Table& /*table*/,
                                                                  const api::tesseract::ExtractResult& ocr,
                                                                  const std::string& pageCropImagePath,
                                                                  std::shared_ptr<api::opencv::IOpenCvService> opencv,
                                                                  const std::vector<uint8_t>& pageCropImageBytes,
                                                                  const std::filesystem::path& proofOutputDir,
                                                                  std::string initialBookingDate,
                                                                  int initialTransactionIndex) {
    ParseResult out;

    out.debugLines.push_back(std::string("pageCropImagePath\t") + pageCropImagePath);
    out.debugLines.push_back(std::string("pageCropImageBytes\t") + std::to_string(pageCropImageBytes.size()));
    try { out.debugLines.push_back(std::string("proofOutputDir\t") + proofOutputDir.string()); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::proofOutputDir", std::current_exception()); }
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

    const auto merged = selectiveGroupMergeLinesRaw(rawLite, 8, { seed.valutaX, seed.debitX, seed.creditX });
    // convert merged back to RawLine
    std::vector<RawLine> lines; lines.reserve(merged.size());
    for (const auto& ml : merged) lines.push_back(RawLine{0, ml.minX, ml.maxX, ml.minY, ml.maxY, ml.wordSpans, ml.text});
    out.debugLines.push_back(std::string("ocr.lines\t") + std::to_string(lines.size()));

    // OcrLine view for merged raw lines (used by helpers)
    std::vector<OcrLine> ocrFromRaw; ocrFromRaw.reserve(lines.size());
    for (const auto& rl : lines) ocrFromRaw.push_back(rawToOcrLine(rl));

    // Collect page-wide header booking dates from merged lines so we can
    // update `currentBookingDate` as we iterate through the page.
    std::vector<std::pair<size_t,std::string>> pageHeaderDates;
    try {
        for (size_t i = 0; i < lines.size(); ++i) {
            try {
                if (auto bd = findBookingDateInHeader(lines[i].text)) {
                    pageHeaderDates.emplace_back(i, *bd);
                }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerOverrideByValutaAnchor", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::pageHeaderDates", std::current_exception()); }

    std::string currentBookingDate = std::move(initialBookingDate);
    int txIndex = std::max(1, initialTransactionIndex);

    size_t headerScanPtr = 0; // pointer into pageHeaderDates

    // Early empty-page detection: centralized helper
    {
        std::string dbg;
        if (detectEarlyEmptyPage(ocrLines, dbg)) {
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

    const HeaderAnalysis headerAnalysis = analyzeHeaderWindow(ocrLines, lines, seedCols, out);
    static constexpr size_t headerScanLines = 12;
    int headerBottomY = headerAnalysis.headerBottomY;
    const int headerMarginPx = headerAnalysis.headerMarginPx;
    const int pageMaxY = headerAnalysis.pageMaxY;
    const bool headerFound = headerAnalysis.headerFound;

    // Booking-date fallback: try centralized helper
    if (currentBookingDate.empty()) {
        if (auto d = findFallbackBookingDate(ocrLines, 30)) {
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

        // Update currentBookingDate if this line index matches a saved header date index
        if (headerScanPtr < pageHeaderDates.size() && pageHeaderDates[headerScanPtr].first == li) {
            // this exact line is a detected header 'Buchungsdatum' -> treat as header, not a transaction
            currentBookingDate = pageHeaderDates[headerScanPtr].second;
            out.debugLines.push_back(std::string("header.foundOnPage\tline=") + std::to_string(li) + std::string("\t") + currentBookingDate);
            ++headerScanPtr;
            // ensure following lines are considered transactions but do not parse this header line as transaction
            inTransactions = true;
            flush();
            cur.bookingDateGroup = currentBookingDate;
            prevLine = txt;
            continue;
        }
        // advance any header pointers that are before current index (defensive)
        while (headerScanPtr < pageHeaderDates.size() && pageHeaderDates[headerScanPtr].first < li) ++headerScanPtr;

        if (isFooterLine(txt)) { out.debugLines.push_back(std::string("stop.footer\t") + txt + "\tline=" + std::to_string(li)); break; }
        // Only treat post-transaction footnote as terminal if we already started parsing transactions
        // or have found blocks, or the footnote lies near the bottom of the page. This avoids
        // prematurely stopping when header-like lines appear near the header bottom.
        if (isPostTransactionFootnote(txt)) {
            bool nearBottom = false;
            try { if (pageMaxY >= 0 && l.maxY >= static_cast<int>(pageMaxY * 3 / 4)) nearBottom = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::footnoteNearBottom", std::current_exception()); }
            // Only treat as terminal when we already have found transaction blocks and
            // the footnote is clearly near the bottom of the page. This avoids
            // aborting pages where an initial booking date is provided but no
            // transactions have yet been detected on this page.
            try {
                out.debugLines.push_back(std::string("footnote.check\tpageMaxY=") + std::to_string(pageMaxY) + std::string("\tlineY=") + std::to_string(l.maxY) + std::string("\tnearBottom=") + (nearBottom ? "1" : "0") + std::string("\tblocks=") + std::to_string(blocks.size()));
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::valuta", std::current_exception()); }

            bool reallyTerminal = false;
            try { if (!blocks.empty() && nearBottom) reallyTerminal = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::footnoteTerminal", std::current_exception()); }
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
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerOverrideByValutaAnchor", std::current_exception()); }

            // Do not skip lines that contain explicit header signals such as a booking date,
            // 'valuta' or any full date token. This prevents legitimate header/data lines
            // from being swallowed by conservative header bounding.
            bool hasHeaderSignal = false;
            try {
                if (findBookingDateInHeader(txt).has_value()) hasHeaderSignal = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::txt", std::current_exception()); }
            try {
                // also consider the combined previous+current token for split OCR cases
                if (findBookingDateInHeader(combined).has_value()) hasHeaderSignal = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::combined", std::current_exception()); }
            try {
                if (isValutaHeaderLine(txt)) hasHeaderSignal = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::valuta", std::current_exception()); }

            if (!hasHeaderSignal && !(rows.inSection && cols.hasValuta())) {
                // before skipping, attempt combined prev+curr / curr+next main-row detection
                bool combinedStarted = false;
                try {
                    std::string combPrev = prevLine.empty() ? txt : (prevLine + std::string(" ") + txt);
                    std::string combNext = txt;
                    if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li+1].text;
                    try {
                        if (auto cs = tryCombinedStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true;
                            out.debugLines.push_back(std::string("tx.start.combined.aboveHeader\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                            combinedStarted = true;
                        }
                    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::combinedStartInner", std::current_exception()); }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::combinedStart", std::current_exception()); }
                if (combinedStarted) {
                    // allow processing as normal
                } else {
                    // If the line contains an amount-like token or matches a loose transaction pattern,
                    // allow it to start transactions even though it's above the conservative header bound.
                    try {
                        core::parser::OcrLine ol = rawToOcrLine(l);
                        bool allowAboveHeader = false;
                        try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) allowAboveHeader = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::allowAboveHeader::hasAmount", std::current_exception()); }
                        try { if (!allowAboveHeader && core::parser::helpers::isLooseTransactionLine(ol, cols.valutaX)) allowAboveHeader = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::allowAboveHeader::isLoose", std::current_exception()); }
                        if (allowAboveHeader) {
                            inTransactions = true;
                            ++txStartLooseCount;
                            out.debugLines.push_back(std::string("tx.start.aboveHeaderDetected\tline=") + std::to_string(li) + "\ttext=" + txt);
                        } else {
                            out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                            prevLine = txt;
                            continue;
                        }
                    } catch (...) {
                        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::allowAboveHeader", std::current_exception());
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
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerBottomY::debitCredit", std::current_exception()); }
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
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerBottomY::section", std::current_exception()); }
            prevLine = txt;
            continue;
        }

        if (auto bd = findBookingDateInHeader(txt)) {
            currentBookingDate = *bd;
            out.debugLines.push_back(std::string("header.bookingDate\t") + currentBookingDate + "\tline=" + std::to_string(li));
            flush();
            cur.bookingDateGroup = currentBookingDate;
            // update header bottom
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerBottomY::bookingDate", std::current_exception()); }
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
                try { if (seedCols.valutaX >= 0) { core::parser::OcrLine _ol = rawToOcrLine(l); if (core::parser::helpers::hasTokenNearX(_ol, seedCols.valutaX, core::parser::helpers::parserConfig.tokenNearBandForMainRow) || core::parser::helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) anchoredToValuta = true; } } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerNoise::anchoredToValuta", std::current_exception()); }

                bool looksMain = false;
                try {
                    core::parser::OcrLine _ol = rawToOcrLine(l);
                    if (core::parser::helpers::isLooseTransactionLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (core::parser::helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (core::parser::helpers::hasShortDateToken(_ol.text) && core::parser::helpers::hasLeftDescriptiveText(_ol, seedCols.valutaX)) looksMain = true;
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerNoise::looksMain", std::current_exception()); }

                if (!anchoredToValuta && !looksMain && !isTransactionsSectionHeader(txt) && !isDebitCreditHeaderLine(txt) && !isLikelyTransactionMainRowText(txt)) {
                    out.debugLines.push_back(std::string("line.skip.headerNoise\t") + txt + std::string("\tline=") + std::to_string(li));
                    prevLine = txt;
                    headerNoiseSkipped = true;
                } else {
                    out.debugLines.push_back(std::string("header.noise_filter.suppressed\tline=") + std::to_string(li));
                }
            }
        } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerNoise", std::current_exception()); }
        if (headerNoiseSkipped) continue;

        if (!inTransactions) {
            const bool canStart = (!currentBookingDate.empty()) || (rows.inSection && cols.hasValuta());
            bool didVerticalStart = false;
            // conservative vertical merge: check neighbor lines for amount/date relationship
            try {
                if (canStart && rows.inSection && cols.hasValuta()) {
                    if (auto vs = tryVerticalStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
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
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::verticalStart", std::current_exception()); }

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
                        if (auto cs = tryCombinedStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true; startedCombined = true;
                            out.debugLines.push_back(std::string("tx.start.combined\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                        }
                    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::combinedStartInner", std::current_exception()); }
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::combinedStart", std::current_exception()); }
                if (startedCombined) {
                    // continue and let normal main detection handle the line
                } else {
                    // Additional relaxed start heuristic: if the line contains a short date token and an amount-like token
                    // (or descriptive left text), treat it as a transaction start. Also check neighboring lines for amount tokens.
                    bool relaxedStart = false;
                    try {
                        core::parser::OcrLine ol = rawToOcrLine(l);
                        bool hasDate = false;
                        try { if (core::parser::helpers::hasShortDateToken(ol.text)) hasDate = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::relaxedStart::hasShortDate", std::current_exception()); }
                        bool hasAmount = false;
                        try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) hasAmount = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::relaxedStart::hasAmount", std::current_exception()); }
                        // check previous and next lines for amount tokens as fallback
                        try {
                            if (!hasAmount && li > 0) { core::parser::OcrLine prevOl = rawToOcrLine(lines[li-1]); if (core::parser::helpers::hasAmountLikeTokenInLine(prevOl, cols.valutaX)) hasAmount = true; }
                            if (!hasAmount && li + 1 < lines.size()) { core::parser::OcrLine nextOl = rawToOcrLine(lines[li+1]); if (core::parser::helpers::hasAmountLikeTokenInLine(nextOl, cols.valutaX)) hasAmount = true; }
                        } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::relaxedStart::neighborAmount", std::current_exception()); }
                        bool hasLeftDesc = false;
                        try { if (core::parser::helpers::hasLeftDescriptiveText(ol, cols.valutaX)) hasLeftDesc = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::relaxedStart::hasLeftDesc", std::current_exception()); }
                        if (hasDate && (hasAmount || hasLeftDesc)) relaxedStart = true;
                    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::relaxedStart", std::current_exception()); }
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
                    cur.main = handleMainRow(rawToOcrLine(l), cg, false, out.debugLines);
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
            cur.main = handleMainRow(rawToOcrLine(l), cg, mainByGeom, out.debugLines);
            prevLine = txt;
            continue;
        }

        if (!cur.main.left.empty()) {
            core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
            appendDetailLine(cur, rawToOcrLine(l), cg, &out.debugLines);
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
                try { if (core::parser::helpers::hasAmountLikeTokenInLine(ol, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::hasAmount", std::current_exception()); }
                try { if (!looksMain && core::parser::helpers::isLooseTransactionLine(ol, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::isLoose", std::current_exception()); }
                try { if (!looksMain && core::parser::helpers::hasShortDateToken(ol.text) && core::parser::helpers::hasLeftDescriptiveText(ol, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::dateAndLeft", std::current_exception()); }
                if (looksMain) {
                    TransactionBlock nb;
                    nb.bookingDateGroup = currentBookingDate;
                    core::parser::helpers::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
                    nb.main = handleMainRow(ol, cg, false, out.debugLines);
                    blocks.push_back(std::move(nb));
                    out.debugLines.push_back(std::string("tx.start.rescued\ttext=") + ol.text);
                }
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::loop", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue", std::current_exception()); }

    attachOrphansToBlocks(blocks,
                          orphanLines,
                          core::constants::parser::kAttachOrphanMaxDistance,
                          cols.valutaX,
                          &out.debugLines);

    out.debugLines.push_back(std::string("blocks\t") + std::to_string(blocks.size()));

    appendPageSummary(lines, headerBottomY, blocks, txStartLooseCount, out);
    appendTransactionsFromBlocks(blocks, cols, ocr, opencv, pageCropImagePath, pageCropImageBytes, proofOutputDir, txIndex, out);

    out.debugLines.push_back(std::string("transactions\t") + std::to_string(out.transactions.size()));

    out.lastBookingDate = currentBookingDate;
    out.nextTransactionIndex = txIndex;
    return out;
}
