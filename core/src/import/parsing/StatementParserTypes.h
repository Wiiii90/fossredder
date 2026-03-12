/**
 * @file core/src/import/parsing/StatementParserTypes.h
 * @brief Declares private statement-parser support types shared across parser implementation files.
 */

#pragma once

#include <limits>
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

} // namespace core::parser::detail
