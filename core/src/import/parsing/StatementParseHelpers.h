/**
 * @file core/src/import/parsing/StatementParseHelpers.h
 * @brief Declares private helper types and routines shared by statement parsing stages.
 */

#pragma once

#include "DefaultStatementParser.h"
#include "ParserTypes.h"
#include "StatementParserTypes.h"

#include "ParserHelpers.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/models/TransactionDraft.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace core::parser::detail {

OcrLine rawToOcrLine(const RawLine& line);
bool isLikelyTransactionMainRowGeom(const RawLine& line, const ColumnModel& cols);
bool isLikelyTransactionHeaderLine(const OcrLine& line, const ColumnModel& cols);
std::pair<int, bool> detectHeaderRegion(const std::vector<OcrLine>& lines, size_t scanLines);
std::optional<std::string> findFallbackBookingDate(const std::vector<OcrLine>& lines, size_t scanLines) noexcept;
std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept;
bool detectEarlyEmptyPage(const std::vector<OcrLine>& lines, std::string& outDebug) noexcept;
std::vector<helpers::detail::RawLineLite> selectiveGroupMergeLinesRaw(const std::vector<helpers::detail::RawLineLite>& lines,
                                                                      int maxGapPx,
                                                                      const helpers::ColumnGuess& seedCols);
std::optional<std::pair<TransactionMainRow, int>> tryVerticalStart(const std::vector<OcrLine>& lines,
                                                                   size_t li,
                                                                   const helpers::ColumnGuess& cols) noexcept;
std::optional<std::pair<TransactionMainRow, int>> tryCombinedStart(const std::vector<OcrLine>& lines,
                                                                   size_t li,
                                                                   const helpers::ColumnGuess& cols) noexcept;
void appendDetailLine(TransactionBlock& cur,
                      const OcrLine& line,
                      const helpers::ColumnGuess& cols,
                      std::vector<std::string>* debugOut = nullptr) noexcept;
TransactionMainRow handleMainRow(const OcrLine& line,
                                 const helpers::ColumnGuess& cols,
                                 bool isGeom,
                                 std::vector<std::string>& debugOut) noexcept;
void attachOrphansToBlocks(std::vector<TransactionBlock>& blocks,
                           const std::vector<OcrLine>& orphans,
                           int maxGapPx,
                           int valutaX,
                           std::vector<std::string>* debugOut = nullptr) noexcept;
HeaderAnalysis analyzeHeaderWindow(const std::vector<OcrLine>& ocrLines,
                                   const std::vector<RawLine>& lines,
                                   const ColumnModel& seedCols,
                                   DefaultStatementParser::ParseResult& out);
void rescueOrphanBlocks(std::vector<TransactionBlock>& blocks,
                        const std::vector<OcrLine>& orphanLines,
                        const ColumnModel& cols,
                        const std::string& currentBookingDate,
                        DefaultStatementParser::ParseResult& out);
void appendPageSummary(const std::vector<RawLine>& lines,
                       int headerBottomY,
                       const std::vector<TransactionBlock>& blocks,
                       int txStartLooseCount,
                       DefaultStatementParser::ParseResult& out);
void appendTransactionsFromBlocks(const std::vector<TransactionBlock>& blocks,
                                  const ColumnModel& cols,
                                  const api::tesseract::ExtractResult& ocr,
                                  const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                  const std::string& pageCropImagePath,
                                  const std::vector<uint8_t>& pageCropImageBytes,
                                  const std::filesystem::path& proofOutputDir,
                                  int& txIndex,
                                  DefaultStatementParser::ParseResult& out);

} // namespace core::parser::detail
