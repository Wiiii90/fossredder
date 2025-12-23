#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include <memory>
#include <opencv2/opencv.hpp>

class IDebugger;

namespace opencv {

class MaskEngine {
public:
    // Apply mask/inpaint to an image on disk.
    static api::opencv::MaskResult MaskImage(const api::opencv::MaskRequest& req, std::shared_ptr<IDebugger> debugger = nullptr);

    // Utility: mask rectangles on an existing grayscale image (used by detection pipelines).
    static cv::Mat ApplyTextMasks(const cv::Mat& gray, const std::vector<cv::Rect>& rects, int padX, int padY);
};

} // namespace opencv
