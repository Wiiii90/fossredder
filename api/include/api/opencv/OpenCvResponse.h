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
};

struct DetectResult {
    // return single table (or none) instead of list
    Table table;
    bool detected = false;
};

struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
};

}