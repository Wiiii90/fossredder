#pragma once

#include <string>
#include <optional>
#include <vector>
#include "api/tesseract/TesseractResponse.h"
#include "core/parser/ParserHeuristics.h"
#include <regex>

namespace core::parser {
struct OcrLine;
struct TransactionMainRow;
struct TransactionBlock;
}

namespace core::parser::helpers {

// Centralized parser configuration for tuning heuristics and thresholds
struct ParserConfig {
    int amountNearValutaBandPx = 120;         // used when searching for amount tokens near valuta
    int valutaNeighborExpandPx = 140;         // expand window to include adjacent token as valuta
    int leftDescriptiveOffsetPx = 200;        // X offset to consider token as left descriptive text
    int tokenNearMergeBandPx = 220;           // band used when checking tokens near valuta for merging
    int maxPhraseTokens = 6;                  // max tokens to consider when matching a phrase

    int tokenNearBandForMainRow = 100;        // band used in geom-based main row detection
    int groupMergeMaxGapPx = 8;               // default max vertical gap for grouping lines
    int headerScanLines = 12;                 // number of lines scanned for header heuristics
    int headerMarginPx = 8;                   // header margin when skipping lines above header
    int orphanAttachMaxGapPx = 80;            // max gap for attaching orphan lines to blocks
    int orphanAcceptXOffsetPx = 40;           // allow orphans whose maxX < valutaX + offset
};

extern ParserConfig parserConfig;

// Consolidated amount helper: return indices of tokens that look like amounts in the line
// If valutaX>=0 and bandPx>0, restrict search to tokens near that X (within bandPx)
std::vector<size_t> findAmountTokenIndices(const core::parser::OcrLine& line, int valutaX = -1, int bandPx = 0) noexcept;

// Short date presence check
bool hasShortDateToken(const std::string& text) noexcept;

// Check whether a text contains a short date token like "DD. MM" or "DD.MM"
bool containsShortDate(const std::string& text) noexcept;

// Find first full date (DD.MM.YYYY) in text, returns string if found
std::optional<std::string> findFirstFullDate(const std::string& text) noexcept;

// Normalize a string to alphanumeric lowercase (a-z0-9)
std::string normalizeAlnumLower(const std::string& s) noexcept;

// Token/phrase spatial helpers
std::optional<int> findTokenCenterX(const core::parser::OcrLine& line, const std::string& tokenLower) noexcept;
std::optional<int> findPhraseCenterX(const core::parser::OcrLine& line, std::initializer_list<const char*> phraseLower) noexcept;

// Check if any token in the line appears near x within bandPx
bool hasTokenNearX(const core::parser::OcrLine& line, int x, int bandPx) noexcept;

// Line-level helpers used by statement parser (operate on OcrLine)
bool hasAmountLikeTokenInLine(const core::parser::OcrLine& line, int valutaX) noexcept;
bool hasLeftDescriptiveText(const core::parser::OcrLine& line, int valutaX) noexcept;
bool hasAmountNearValuta(const core::parser::OcrLine& line, int valutaX, int bandPx) noexcept;
bool isLooseTransactionLine(const core::parser::OcrLine& line, int valutaX) noexcept;

// Bulk helpers: build OcrLine list and infer columns
namespace detail {
    struct RawLineLite { int minX; int maxX; int minY; int maxY; std::vector<std::pair<int,int>> wordSpans; std::string text; };
}

// Build OcrLine vector from tesseract words (wrapper over existing logic)
std::vector<core::parser::OcrLine> buildOcrLinesFromWords(const std::vector<api::tesseract::Word>& words);

// Infer ColumnModel-like result (valuta/debit/credit X positions) from initial raw OcrLines
struct ColumnGuess { int valutaX = -1; int debitX = -1; int creditX = -1; };
ColumnGuess inferColumnModelFromLines(const std::vector<core::parser::OcrLine>& lines, size_t scanN=16) noexcept;

// Grouping helpers that operate on RawLine-like containers
std::vector<detail::RawLineLite> groupMergeLinesRaw(const std::vector<detail::RawLineLite>& lines, int maxGapPx);
std::vector<detail::RawLineLite> selectiveGroupMergeLinesRaw(const std::vector<detail::RawLineLite>& lines, int maxGapPx, const ColumnGuess& seedCols);

// Convert a slice of a RawLineLite's words into an OcrLine (used by splitMainRow)
core::parser::OcrLine toOcrLineFromRawWords(const detail::RawLineLite& src, size_t i0, size_t i1) noexcept;

// Split a raw line into TransactionMainRow (left/valuta/debit/credit) using inferred column X positions
core::parser::TransactionMainRow splitMainRowFromRaw(const detail::RawLineLite& src, int valutaX, int debitX, int creditX) noexcept;

// Header detection: returns preHeaderBottomY (largest maxY of header-like lines) and whether any header signals found
std::pair<int,bool> detectHeaderRegion(const std::vector<core::parser::OcrLine>& lines, size_t scanLines);

// Fallback booking date scan in header area
std::optional<std::string> findFallbackBookingDate(const std::vector<core::parser::OcrLine>& lines, size_t scanLines) noexcept;

// Find booking date after 'buchungsdatum' like header keyword or any full date in the given line
std::optional<std::string> findBookingDateInHeader(const std::string& line) noexcept;

// Early empty-page detection based on bottom lines footnote/amount counts
bool detectEarlyEmptyPage(const std::vector<core::parser::OcrLine>& lines, std::string& outDebug, std::string& foundBookingDate) noexcept;

// Attach orphan lines to nearest preceding main block
void attachOrphansToBlocks(std::vector<core::parser::TransactionBlock>& blocks, const std::vector<core::parser::OcrLine>& orphans, int maxGapPx, int valutaX) noexcept;

// Split main row from an OcrLine using column X positions
core::parser::TransactionMainRow splitMainRowFromOcrLine(const core::parser::OcrLine& src, int valutaX, int debitX, int creditX) noexcept;

// Try vertical-start detection at index li in lines; returns optional<pair<mainRow, consumedLines>>
std::optional<std::pair<core::parser::TransactionMainRow,int>> tryVerticalStart(const std::vector<core::parser::OcrLine>& lines, size_t li, const ColumnGuess& cols, const std::string& currentBookingDate) noexcept;

// Try combined prev+curr / curr+next regex-based start detection
std::optional<std::pair<core::parser::TransactionMainRow,int>> tryCombinedStart(const std::vector<core::parser::OcrLine>& lines, size_t li, const ColumnGuess& cols) noexcept;

// Append a line as detail to a transaction block using the same heuristics as the parser
void appendDetailLine(core::parser::TransactionBlock& cur, const core::parser::OcrLine& l, const ColumnGuess& cols, std::vector<std::string>* debugOut = nullptr) noexcept;

// Handle main-row detection, splitting and debug logging. 'isGeom' indicates geometry-based detection.
core::parser::TransactionMainRow handleMainRow(const core::parser::OcrLine& line, const ColumnGuess& cols, bool isGeom, std::vector<std::string>& debugOut) noexcept;
} // namespace core::parser::helpers
