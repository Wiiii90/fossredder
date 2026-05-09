/**
 * @file core/include/core/ports/image-processing/OpenCvResult.h
 * @brief Result types for image processing operations.
 */
#pragma once

#include "core/ports/image-processing/OpenCvTypes.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace core::ports::image_processing::opencv {

/**
 * @brief Result of denoising.
 */
struct DenoiseResult {
    std::filesystem::path denoisedImagePath;
};

/**
 * @brief Result of masking.
 */
struct MaskResult {
    std::filesystem::path maskedImagePath;
    std::filesystem::path maskImagePath;
    std::vector<uint8_t> maskedImageBytes;
    std::vector<uint8_t> maskImageBytes;
};

/**
 * @brief Result of detection.
 */
struct DetectResult {
    Table table;
    bool detected = false;
    std::vector<Rect> textBlocks;
};

/**
 * @brief Result of cropping.
 */
struct CropResult {
    std::vector<std::filesystem::path> croppedImagePaths;
    std::vector<std::vector<uint8_t>> croppedImageBytes;
};

} // namespace core::ports::image_processing::opencv
