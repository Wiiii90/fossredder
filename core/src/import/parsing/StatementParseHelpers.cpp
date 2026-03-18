/**
 * @file core/src/import/parsing/StatementParseHelpers.cpp
 * @brief Implements helper types and routines shared by statement parsing stages.
 */

#include "StatementParseHelpers.h"

#include "AmountParser.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "DefaultTransactionParser.h"
#include "ParserHelpers.h"
#include "../../utils/UniqId.h"

#include <algorithm>
#include <cmath>

namespace core::parser::detail {

namespace {

template <typename Action>
bool tryReportParserWarning(const char* origin, Action&& action)
{
    try {
        std::forward<Action>(action)();
        return true;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, origin, std::current_exception());
        return false;
    }
}

}

OcrLine rawToOcrLine(const RawLine& line) {
    OcrLine out;
    out.minX = line.minX;
    out.maxX = line.maxX;
    out.minY = line.minY;
    out.maxY = line.maxY;
    out.wordSpans = line.wordSpans;
    out.text = line.text;
    return out;
}

bool isLikelyTransactionMainRowGeom(const RawLine& line, const ColumnModel& cols) {
    if (!cols.hasValuta() || (!cols.hasDebit() && !cols.hasCredit())) return false;
    const auto tokens = utils::splitWhitespace(line.text);
    if (tokens.size() != line.wordSpans.size() || tokens.size() < 3) return false;

    const int band = core::parser::helpers::parserConfig.tokenNearBandForMainRow;
    const bool hasValuta = core::parser::helpers::hasTokenNearX(rawToOcrLine(line), cols.valutaX, band);
    bool hasDebit = cols.hasDebit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(line), cols.debitX, band) : false;
    bool hasCredit = cols.hasCredit() ? core::parser::helpers::hasTokenNearX(rawToOcrLine(line), cols.creditX, band) : false;

    if (!hasDebit && !hasCredit) {
        tryReportParserWarning("core::parser::DefaultStatementParser::isLikelyTransactionMainRowGeom", [&] {
            auto amountIndices = core::parser::helpers::findAmountTokenIndices(rawToOcrLine(line), cols.valutaX, core::parser::helpers::parserConfig.amountNearValutaBandPx);
            if (!amountIndices.empty()) hasDebit = true;
        });
    }

    bool hasLeft = false;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& span = line.wordSpans[i];
        const int centerX = (span.first + span.second) / 2;
        if (centerX < cols.valutaX - core::parser::helpers::parserConfig.leftDescriptiveOffsetPx) {
            hasLeft = true;
            break;
        }
    }

    return hasLeft && hasValuta && (hasDebit || hasCredit);
}

bool isLikelyTransactionHeaderLine(const OcrLine& line, const ColumnModel& cols) {
    return cols.hasValuta() && core::parser::helpers::hasTokenNearX(line, cols.valutaX, 100);
}

HeaderAnalysis analyzeHeaderWindow(const std::vector<OcrLine>& ocrLines,
                                   const std::vector<RawLine>& lines,
                                   const ColumnModel& seedCols,
                                   DefaultStatementParser::ParseResult& out)
{
    HeaderAnalysis result;
    static constexpr size_t headerScanLines = 12;

    auto [preHeaderBottomY, headerFound] = core::parser::helpers::detectHeaderRegion(ocrLines, headerScanLines, seedCols.valutaX);
    result.preHeaderBottomY = preHeaderBottomY;
    result.headerFound = headerFound;
    result.headerBottomY = preHeaderBottomY;

    out.debugLines.push_back(std::string("header.prebottomY\t") + std::to_string(preHeaderBottomY));
    out.debugLines.push_back(std::string("header.found\t") + (headerFound ? "1" : "0"));

    try {
        const size_t debugCount = std::min(ocrLines.size(), headerScanLines);
        for (size_t headerIndex = 0; headerIndex < debugCount; ++headerIndex) {
            const auto& line = ocrLines[headerIndex];
            const auto& text = line.text;
            try {
                const bool isTxSection = core::parser::heuristics::isTransactionsSectionHeader(text);
                const bool isDebitCredit = core::parser::heuristics::isDebitCreditHeaderLine(text);
                const bool isNoise = core::parser::heuristics::isHeaderNoiseLine(text);
                const bool hasFullDate = core::parser::helpers::findFirstFullDate(text).has_value();
                bool hasAmount = core::parser::helpers::hasAmountLikeTokenInLine(line, seedCols.valutaX);
                bool hasValutaTokenNear = false;
                tryReportParserWarning("core::parser::DefaultStatementParser::headerCandidateValutaNear", [&] {
                    if (seedCols.valutaX >= 0) hasValutaTokenNear = core::parser::helpers::hasTokenNearX(line, seedCols.valutaX, core::parser::helpers::parserConfig.tokenNearBandForMainRow);
                });
                std::ostringstream summary;
                summary << "header.candidate\tline=" << headerIndex << "\ttext=" << text << "\ttxSection=" << (isTxSection ? "1" : "0") << "\tdebitcredit=" << (isDebitCredit ? "1" : "0") << "\tnoise=" << (isNoise ? "1" : "0") << "\tfullDate=" << (hasFullDate ? "1" : "0") << "\thasAmt=" << (hasAmount ? "1" : "0") << "\tvalutaNear=" << (hasValutaTokenNear ? "1" : "0");
                out.debugLines.push_back(summary.str());
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::txt", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerCandidates", std::current_exception()); }

    try {
        int earliestAmountY = std::numeric_limits<int>::max();
        bool foundAmountLine = false;
        for (const auto& line : lines) {
            try {
                OcrLine ocrLine = rawToOcrLine(line);
                const bool hasAmount = core::parser::helpers::hasAmountLikeTokenInLine(ocrLine, seedCols.valutaX);
                const bool dateLeft = core::parser::helpers::hasShortDateToken(ocrLine.text) && core::parser::helpers::hasLeftDescriptiveText(ocrLine, seedCols.valutaX);
                if (!hasAmount && !dateLeft) continue;
                earliestAmountY = std::min(earliestAmountY, line.maxY);
                foundAmountLine = true;
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::hasHeaderSignal::combined", std::current_exception()); }
        }
        if (foundAmountLine && earliestAmountY != std::numeric_limits<int>::max()) {
            result.headerBottomY = std::min(result.headerBottomY, earliestAmountY - 2);
            out.debugLines.push_back(std::string("header.adjustedForEarliestAmount\t") + std::to_string(result.headerBottomY));
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::headerAdjustEarliestAmount", std::current_exception()); }

    try {
        for (const auto& line : lines) result.pageMaxY = std::max(result.pageMaxY, line.maxY);
    } catch (...) { result.pageMaxY = -1; }

    return result;
}

static bool looksLikeRescuedMainLine(const OcrLine& line, const ColumnModel& cols)
{
    bool looksMain = false;
    try { if (core::parser::helpers::hasAmountLikeTokenInLine(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::hasAmount", std::current_exception()); }
    try { if (!looksMain && core::parser::helpers::isLooseTransactionLine(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::isLoose", std::current_exception()); }
    try { if (!looksMain && core::parser::helpers::hasShortDateToken(line.text) && core::parser::helpers::hasLeftDescriptiveText(line, cols.valutaX)) looksMain = true; } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::dateAndLeft", std::current_exception()); }
    return looksMain;
}

void rescueOrphanBlocks(std::vector<TransactionBlock>& blocks,
                        const std::vector<OcrLine>& orphanLines,
                        const ColumnModel& cols,
                        const std::string& currentBookingDate,
                        DefaultStatementParser::ParseResult& out)
{
    try {
        for (const auto& orphanLine : orphanLines) {
            try {
                if (!looksLikeRescuedMainLine(orphanLine, cols)) continue;
                TransactionBlock block;
                block.bookingDateGroup = currentBookingDate;
                core::parser::helpers::ColumnGuess guess{ cols.valutaX, cols.debitX, cols.creditX };
                block.main = core::parser::helpers::handleMainRow(orphanLine, guess, false, out.debugLines);
                blocks.push_back(std::move(block));
                out.debugLines.push_back(std::string("tx.start.rescued\ttext=") + orphanLine.text);
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue::loop", std::current_exception()); }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::rescue", std::current_exception()); }
}

void appendPageSummary(const std::vector<RawLine>& lines,
                       int headerBottomY,
                       const std::vector<TransactionBlock>& blocks,
                       int txStartLooseCount,
                       DefaultStatementParser::ParseResult& out)
{
    try {
        std::ostringstream summary;
        summary << "page.summary\tocr.lines=" << lines.size() << "\theader.prebottomY=" << headerBottomY << "\tblocks=" << blocks.size() << "\ttxStartLoose=" << txStartLooseCount;
        out.debugLines.push_back(summary.str());
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::pageSummary", std::current_exception()); }
}

static void appendBlockDebug(const TransactionBlock& block,
                             const ColumnModel& cols,
                             DefaultStatementParser::ParseResult& out)
{
    try {
        out.debugLines.push_back(std::string("block.debug\tbookingDateGroup=") + block.bookingDateGroup);
        out.debugLines.push_back(std::string("block.main.left.text\t") + block.main.left.line.text);
        out.debugLines.push_back(std::string("block.main.valuta.empty\t") + (block.main.valuta.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.valuta.text\t") + (block.main.valuta.empty() ? std::string("(none)") : block.main.valuta.line.text));
        out.debugLines.push_back(std::string("block.main.debit.empty\t") + (block.main.debit.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.debit.text\t") + (block.main.debit.empty() ? std::string("(none)") : block.main.debit.line.text));
        out.debugLines.push_back(std::string("block.main.credit.empty\t") + (block.main.credit.empty() ? "1" : "0"));
        out.debugLines.push_back(std::string("block.main.credit.text\t") + (block.main.credit.empty() ? std::string("(none)") : block.main.credit.line.text));
        out.debugLines.push_back(std::string("cols.state\tvalutaX=") + std::to_string(cols.valutaX) + std::string("\tdebitX=") + std::to_string(cols.debitX) + std::string("\tcreditX=") + std::to_string(cols.creditX));
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::blockDebug", std::current_exception()); }
}

static void applyCellAmountOverride(ImportedTransaction& tx,
                                    const TransactionBlock& block,
                                    const ColumnModel& cols,
                                    const api::tesseract::ExtractResult& ocr,
                                    DefaultStatementParser::ParseResult& out)
{
    try {
        if (ocr.tables.empty() || ocr.tables[0].cells.empty()) return;

        const auto& table = ocr.tables[0];
        auto overlapsLine = [&](const api::tesseract::Cell& cell) -> bool {
            const int cellTop = cell.bbox.y;
            const int cellBottom = cell.bbox.y + cell.bbox.height;
            return !(cellTop > block.main.left.line.maxY || cellBottom < block.main.left.line.minY);
        };

        bool usedCell = false;
        if (cols.hasCredit() && cols.creditCol >= 0) {
            for (const auto& cell : table.cells) {
                if (cell.col != cols.creditCol || !overlapsLine(cell)) continue;
                out.debugLines.push_back(std::string("cell.amount_used\t") + cell.text);
                if (auto value = core::parser::parseAmountString(cell.text)) {
                    tx.amount = *value;
                    usedCell = true;
                    out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount));
                }
                break;
            }
        }

        if (!usedCell && cols.hasDebit() && cols.debitCol >= 0) {
            for (const auto& cell : table.cells) {
                if (cell.col != cols.debitCol || !overlapsLine(cell)) continue;
                out.debugLines.push_back(std::string("cell.amount_used\t") + cell.text);
                if (auto value = core::parser::parseAmountString(cell.text)) {
                    tx.amount = -std::abs(*value);
                    out.debugLines.push_back(std::string("cell.override->") + std::to_string(tx.amount));
                }
                break;
            }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::cellOverride", std::current_exception()); }
}

static void attachProofCrop(ImportedTransaction& tx,
                            const TransactionBlock& block,
                            int txIndex,
                            const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                            const std::string& pageCropImagePath,
                            const std::vector<uint8_t>& pageCropImageBytes,
                            const std::filesystem::path& proofOutputDir,
                            DefaultStatementParser::ParseResult& out)
{
    try {
        if (!opencv || (pageCropImagePath.empty() && pageCropImageBytes.empty())) return;

        int minX = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int minY = std::numeric_limits<int>::max();
        int maxY = std::numeric_limits<int>::min();

        auto accBounds = [&](const OcrLine& line) {
            minX = std::min(minX, line.minX);
            maxX = std::max(maxX, line.maxX);
            minY = std::min(minY, line.minY);
            maxY = std::max(maxY, line.maxY);
        };

        if (!block.main.left.empty()) accBounds(block.main.left.line);
        for (const auto& line : block.detailLines) accBounds(line);
        if (minX == std::numeric_limits<int>::max()) return;

        api::opencv::CropRequest request;
        if (!pageCropImagePath.empty()) request.imagePath = std::filesystem::path(pageCropImagePath);
        request.imageBytes = pageCropImageBytes;
        request.outputDir = proofOutputDir;
        request.uniqIdPrefix = std::string(utils::makeUniqId());
        request.filePrefix = std::string("opencv_proof_tx") + std::to_string(txIndex);
        request.outputFormat = api::opencv::CropRequest::OutputFormat::Jpg;
        request.jpegQuality = 92;
        request.bbox.x = 0;
        request.bbox.y = std::max(0, minY - 20);
        request.bbox.width = 1 << 30;
        request.bbox.height = std::max(1, (maxY - minY) + 24);

        try {
            out.debugLines.push_back(std::string("crop.request\t") + request.imagePath.string() + std::string("\tfilePrefix=") + request.filePrefix);
            const auto response = opencv->crop(request);
            out.debugLines.push_back(std::string("crop.result.count\t") + std::to_string(response.croppedImagePaths.size()));
            if (!response.croppedImageBytes.empty() && !response.croppedImageBytes.front().empty()) {
                std::string proofKey = std::string("proof/tx_") + std::string(utils::makeUniqId()) + std::string(".jpg");
                tx.proofImagePath = proofKey;
                out.artifacts.emplace(proofKey, response.croppedImageBytes.front());
            } else if (!response.croppedImagePaths.empty()) {
                try { tx.proofImagePath = std::filesystem::absolute(response.croppedImagePaths.front()).string(); }
                catch (...) { tx.proofImagePath = response.croppedImagePaths.front().string(); }
            }
        } catch (const std::exception& ex) {
            out.debugLines.push_back(std::string("crop.exception\t") + ex.what());
        } catch (...) {
            out.debugLines.push_back(std::string("crop.exception\tunknown"));
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::parser::DefaultStatementParser::proofCrop", std::current_exception()); }
}

void appendTransactionsFromBlocks(const std::vector<TransactionBlock>& blocks,
                                  const ColumnModel& cols,
                                  const api::tesseract::ExtractResult& ocr,
                                  const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                  const std::string& pageCropImagePath,
                                  const std::vector<uint8_t>& pageCropImageBytes,
                                  const std::filesystem::path& proofOutputDir,
                                  int& txIndex,
                                  DefaultStatementParser::ParseResult& out)
{
    for (const auto& block : blocks) {
        if (block.main.left.empty()) continue;

        appendBlockDebug(block, cols, out);

        std::vector<std::string> txDebug;
        const auto parsed = DefaultTransactionParser::parseTransaction(block, &txDebug);
        for (const auto& line : txDebug) out.debugLines.push_back(std::string("txdbg\t") + line);

        ImportedTransaction tx;
        tx.name = "Transaction " + std::to_string(txIndex++);
        tx.bookingDate = parsed.bookingDate;
        tx.valuta = parsed.valuta;
        tx.amount = parsed.amount;
        tx.description = parsed.description;
        tx.metadata = parsed.metadata;

        applyCellAmountOverride(tx, block, cols, ocr, out);
        attachProofCrop(tx, block, txIndex, opencv, pageCropImagePath, pageCropImageBytes, proofOutputDir, out);

        out.transactions.push_back(std::move(tx));
    }
}

}
