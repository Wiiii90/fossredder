#pragma once

#include "core/ports/services/PopplerTypes.h"

#include <filesystem>
#include <vector>

namespace core::ports::services::poppler {

struct RenderResult {
    std::vector<std::filesystem::path> images;
    std::vector<std::vector<uint8_t>> imageBytes;
};

struct ExtractResult {
    std::vector<RenderedPage> pages;
};

} // namespace core::ports::services::poppler

