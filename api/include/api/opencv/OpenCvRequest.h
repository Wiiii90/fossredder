#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "api/opencv/Types.h"

namespace api::opencv {

struct DenoiseRequest {
    std::filesystem::path imagePath;
    enum class Method { Median, Gaussian, Bilateral } method = Method::Median;
};

struct MaskRequest {
    std::filesystem::path imagePath;
    std::vector<Rect> textElements;
    enum class Mode { Whiteout, Inpaint } mode = Mode::Whiteout;
    std::string tesseractTsv;
    bool usePoppler = true;
    bool useTesseract = false;
    bool useMorphology = true;
};

struct DetectRequest {
    std::filesystem::path imagePath;
    enum class DetectKind { Tables, Cells, TextBlocks } kind = DetectKind::Tables;
};

struct CropRequest {
    std::filesystem::path imagePath;
    Rect bbox;
};

}