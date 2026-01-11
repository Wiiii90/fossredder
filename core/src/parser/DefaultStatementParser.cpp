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

    const int band = 100;
    const bool hasValuta = core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.valutaX, band);
    bool hasDebit = cols.hasDebit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.debitX, band) : false;
    bool hasCredit = cols.hasCredit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(l), cols.creditX, band) : false;

    // If neither debit nor credit detected, accept numeric-like token near the valuta column
    if (!hasDebit && !hasCredit) {
        try {
            auto amtIdx = core::parser::helpers::findAmountTokenIndices(rawToOcrLine(l), cols.valutaX, 120);
            if (!amtIdx.empty()) hasDebit = true;
        } catch(...) {}
    }

    bool hasLeft = false;
    for (size_t i = 0; i < toks.size(); ++i) {
        const auto& sp = l.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (cx < cols.valutaX - 200) { hasLeft = true; break; }
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
    ColumnModel cols;
    RowModel rows;

    // Pre-scan lines to determine a conservative header bottom Y so we do not start parsing
    // transactions from text that is clearly part of the page header. This prevents lookahead
    // from combining header lines into a transaction before header detection runs in the loop.
    const size_t headerScanLines = 12; // reduced window for header detection
    auto [preHeaderBottomY, headerFound] = core::parser::helpers::detectHeaderRegion(ocrLines, headerScanLines);
    out.debugLines.push_back(std::string("header.prebottomY\t") + std::to_string(preHeaderBottomY));
    int headerBottomY = preHeaderBottomY; // track lowest (largest y) header line encountered
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
                    out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                    prevLine = txt;
                    continue;
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

        // Header-noise filtering disabled: do not auto-skip lines based on a heuristic that can remove legitimate transactions.
        out.debugLines.push_back(std::string("header.noise_filter.disabled\tline=") + std::to_string(li));

        /*
        if (!inTransactions && !isLikelyTransactionMainRowText(txt) && !isTransactionsSectionHeader(txt) && !isDebitCreditHeaderLine(txt) && isHeaderNoiseLine(txt)) {
            out.debugLines.push_back(std::string("line.skip.headerNoise\t") + txt + "\tline=" + std::to_string(li));
            prevLine = txt;
            continue;
        }
        */

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
                    out.debugLines.push_back(std::string("lookahead.disabled\tline=") + std::to_string(li));
                    out.debugLines.push_back(std::string("line.skip.notInTransactions\t") + txt + "\tline=" + std::to_string(li));
                    // record orphan for post-processing grouping
                    {
                        OcrLine ol;
                        ol.minX = l.minX; ol.maxX = l.maxX; ol.minY = l.minY; ol.maxY = l.maxY; ol.wordSpans = l.wordSpans;
                        ol.text = l.text;
                        orphanLines.push_back(std::move(ol));
                    }
                    prevLine = txt;
                    continue;
                }
            }
        }

        const bool mainByRegex = isLikelyTransactionMainRowText(txt);
        // Only allow geometry-based main-row detection outside the initial header scan window.
        const bool mainByGeom = (!mainByRegex) && (li > headerScanLines) && isLikelyTransactionMainRowGeom(l, cols);

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
