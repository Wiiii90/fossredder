#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct Text {
    std::string text;
    int meanConfidence = 0;
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



