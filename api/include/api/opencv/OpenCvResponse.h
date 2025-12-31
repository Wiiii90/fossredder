#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "api/opencv/Types.h"

namespace api::opencv {

struct DenoiseResult {
    std::filesystem::path denoisedImagePath;
};

struct MaskResult {
    std::filesystem::path maskedImagePath;
    std::filesystem::path maskImagePath;
};

struct DetectResult {
    // return single table (or none) instead of list
    Table table;
    bool detected = false;
    std::vector<Rect> textBlocks; // detected text blocks (morphology)
};

struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
};

}