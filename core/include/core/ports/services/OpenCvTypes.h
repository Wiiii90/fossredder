#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace core::ports::services::opencv {

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct Word {
    Rect bbox;
    std::string text;
    int confidence = 0;
};

struct Cell {
    Rect bbox;
    int row = -1;
    int col = -1;
    std::string text;
    int confidence = 0;
};

struct Table {
    Rect bbox;
    int rows = 0;
    int cols = 0;
    std::vector<Cell> cells;
    std::vector<Word> words;
};

} // namespace core::ports::services::opencv

