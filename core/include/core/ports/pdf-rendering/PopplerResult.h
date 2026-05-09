/**
 * @file core/include/core/ports/pdf-rendering/PopplerResult.h
 * @brief Result types for PDF rendering operations.
 */
#pragma once

#include "core/ports/pdf-rendering/PopplerTypes.h"

#include <filesystem>
#include <vector>

namespace core::ports::pdf_rendering::poppler {

/**
 * @brief Rendering result.
 */
struct RenderResult {
    std::vector<std::filesystem::path> images;
    std::vector<std::vector<uint8_t>> imageBytes;
};

/**
 * @brief Extraction result.
 */
struct ExtractResult {
    std::vector<RenderedPage> pages;
};

} // namespace core::ports::pdf_rendering::poppler
