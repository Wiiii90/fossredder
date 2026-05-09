/**
 * @file core/include/core/ports/image-processing/OpenCvTypes.h
 * @brief Shared geometry and table types for image processing ports.
 */
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace core::ports::image_processing::opencv {

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
 * @brief A recognized word in an image.
 */
struct Word {
    Rect bbox;
    std::string text;
    int confidence = 0;
};

/**
 * @brief A detected table cell.
 */
struct Cell {
    Rect bbox;
    int row = -1;
    int col = -1;
    std::string text;
    int confidence = 0;
};

/**
 * @brief A detected table structure.
 */
struct Table {
    Rect bbox;
    int rows = 0;
    int cols = 0;
    std::vector<Cell> cells;
    std::vector<Word> words;
};

} // namespace core::ports::image_processing::opencv
