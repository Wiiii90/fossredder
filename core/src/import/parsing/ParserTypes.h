/**
 * @file core/src/import/parsing/ParserTypes.h
 * @brief Declares private parser data types shared across parser implementation files.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace core::parser {

struct OcrLine {
    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

struct OcrCell {
    OcrLine line;
    bool empty() const { return line.text.empty(); }
};

struct TransactionMainRow {
    OcrCell left;
    OcrCell valuta;
    OcrCell debit;
    OcrCell credit;
};

struct TransactionBlock {
    std::string bookingDateGroup;
    TransactionMainRow main;
    std::vector<OcrLine> detailLines;
};

} // namespace core::parser
