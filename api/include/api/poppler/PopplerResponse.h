#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "api/poppler/Types.h"

namespace api::poppler {

struct RenderResult {
    std::vector<std::filesystem::path> images;
    std::vector<std::vector<uint8_t>> imageBytes;
};

struct ExtractResult {
    std::vector<RenderedPage> pages;
};

}