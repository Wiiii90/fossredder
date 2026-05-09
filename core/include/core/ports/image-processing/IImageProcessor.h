/**
 * @file core/include/core/ports/image-processing/IImageProcessor.h
 * @brief Image processing port for import workflows.
 */
#pragma once

#include "core/ports/image-processing/OpenCvRequest.h"
#include "core/ports/image-processing/OpenCvResult.h"

namespace core::ports::image_processing {

/**
 * @brief Processes images for the import pipeline.
 */
class IImageProcessor {
public:
    virtual ~IImageProcessor() = default;

    /**
     * @brief Removes noise from an image.
     * @param req Denoising request.
     * @return The denoised image result.
     */
    virtual opencv::DenoiseResult denoise(const opencv::DenoiseRequest& req) const = 0;

    /**
     * @brief Applies a mask to an image.
     * @param req Masking request.
     * @return The masking result.
     */
    virtual opencv::MaskResult mask(const opencv::MaskRequest& req) const = 0;

    /**
     * @brief Detects table or text regions in an image.
     * @param req Detection request.
     * @return The detection result.
     */
    virtual opencv::DetectResult detect(const opencv::DetectRequest& req) const = 0;

    /**
     * @brief Crops an image into one or more regions.
     * @param req Cropping request.
     * @return The cropping result.
     */
    virtual opencv::CropResult crop(const opencv::CropRequest& req) const = 0;
};

} // namespace core::ports::image_processing
