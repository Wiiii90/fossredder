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
    std::vector<Table> tables;
};

struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
};

}