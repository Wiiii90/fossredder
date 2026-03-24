/**
 * @file core/src/import/parsing/ParserHelpers.h
 * @brief Declares private helper types and routines shared within the parser implementation.
 */

#pragma once

#include "ParserConfig.h"
#include "ParserTypes.h"
#include "AmountParser.h"
#include "ParserDateUtils.h"
#include "ParserHeuristics.h"

#include "api/tesseract/TesseractResponse.h"
#include "../../utils/Util.h"

#include <initializer_list>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace core::parser::helpers {

std::vector<size_t> findAmountTokenIndices(const core::parser::OcrLine& line, int valutaX = -1, int bandPx = 0) noexcept;
std::string normalizeAlnumLower(const std::string& s) noexcept;
std::optional<int> findTokenCenterX(const core::parser::OcrLine& line, const std::string& tokenLower) noexcept;
std::optional<int> findPhraseCenterX(const core::parser::OcrLine& line, std::initializer_list<const char*> phraseLower) noexcept;
bool hasTokenNearX(const core::parser::OcrLine& line, int x, int bandPx) noexcept;
bool hasAmountLikeTokenInLine(const core::parser::OcrLine& line, int valutaX) noexcept;
bool hasLeftDescriptiveText(const core::parser::OcrLine& line, int valutaX) noexcept;
bool hasAmountNearValuta(const core::parser::OcrLine& line, int valutaX, int bandPx) noexcept;
bool isLooseTransactionLine(const core::parser::OcrLine& line, int valutaX) noexcept;
std::optional<double> findAndParseAmountInLine(const core::parser::OcrLine& line, int valutaX = -1, std::vector<std::string>* debugOut = nullptr) noexcept;

namespace detail {
    struct RawLineLite {
        int minX;
        int maxX;
        int minY;
        int maxY;
        std::vector<std::pair<int, int>> wordSpans;
        std::string text;
    };
}

std::vector<core::parser::OcrLine> buildOcrLinesFromWords(const std::vector<api::tesseract::Word>& words);

struct ColumnGuess {
    int valutaX = -1;
    int debitX = -1;
    int creditX = -1;
};

ColumnGuess inferColumnModelFromLines(const std::vector<core::parser::OcrLine>& lines, size_t scanN = 16) noexcept;
std::vector<detail::RawLineLite> groupMergeLinesRaw(const std::vector<detail::RawLineLite>& lines, int maxGapPx);
core::parser::OcrLine toOcrLineFromRawWords(const detail::RawLineLite& src, size_t i0, size_t i1) noexcept;
core::parser::TransactionMainRow splitMainRowFromRaw(const detail::RawLineLite& src, int valutaX, int debitX, int creditX) noexcept;
core::parser::TransactionMainRow splitMainRowFromOcrLine(const core::parser::OcrLine& src, int valutaX, int debitX, int creditX) noexcept;
} // namespace core::parser::helpers
