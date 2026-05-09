/**
 * @file core/include/core/ports/text-recognition/TesseractTypes.h
 * @brief Shared OCR data types.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::text_recognition::tesseract {

/**
 * @brief A rectangular region.
 */
struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

/**
 * @brief A recognized text span.
 */
struct Text {
    std::string text;
    int meanConfidence = 0;
};

/**
 * @brief A recognized word.
 */
struct Word {
    Rect bbox;
    std::string text;
    int confidence = 0;
};

/**
 * @brief A recognized table cell.
 */
struct Cell {
    Rect bbox;
    int row = -1;
    int col = -1;
    std::string text;
    int confidence = 0;
};

/**
 * @brief A recognized table structure.
 */
struct Table {
    Rect bbox;
    int rows = 0;
    int cols = 0;
    std::vector<Cell> cells;
    std::vector<Word> words;
};

inline constexpr const char* kDefaultStatementCharWhitelist =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜabcdefghijklmnopqrstuvwxyzäöüß0123456789.,:-/()'“”‘’+&% ";

enum class OcrEngineMode : int {
    LegacyOnly = 0,
    LstmOnly = 1,
    LegacyAndLstm = 2,
    Default = 3
};

} // namespace core::ports::text_recognition::tesseract
