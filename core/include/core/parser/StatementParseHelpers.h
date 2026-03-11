#pragma once

#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/import/ImportedTransaction.h"
#include "core/parser/DefaultStatementParser.h"
#include "core/parser/DefaultTransactionParser.h"

#include <filesystem>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace core::parser::detail {

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

struct HeaderAnalysis {
    int preHeaderBottomY = -1;
    bool headerFound = false;
    int headerBottomY = -1;
    int headerMarginPx = 8;
    int pageMaxY = -1;
};

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

}
