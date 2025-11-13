#pragma once

#include <string>
#include <vector>
#include <filesystem>

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
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
};

struct DenoiseRequest {
    std::filesystem::path imagePath;
    enum class Method { Median, Gaussian, Bilateral } method = Method::Median;
};

struct DenoiseResult {
    std::filesystem::path denoisedImagePath;
};

struct MaskRequest {
    std::filesystem::path imagePath;
    std::vector<Rect> textElements;
    enum class Mode { Whiteout, Inpaint } mode = Mode::Whiteout;
};

struct MaskResult {
    std::filesystem::path maskedImagePath;
};

struct DetectRequest {
    std::filesystem::path imagePath;
    enum class DetectKind { Tables, Cells, TextBlocks } kind = DetectKind::Tables;
};

struct DetectResult {
    std::vector<Table> tables;
};

struct CropRequest {
    std::filesystem::path imagePath;
};

struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
};