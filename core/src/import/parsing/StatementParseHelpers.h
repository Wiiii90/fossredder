/**
 * @file core/src/import/parsing/StatementParseHelpers.h
 * @brief Declares private helper types and routines shared by statement parsing stages.
 */

#pragma once

#include "DefaultStatementParser.h"
#include "ParserTypes.h"
#include "StatementParserTypes.h"

#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/import/ImportedTransaction.h"

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
