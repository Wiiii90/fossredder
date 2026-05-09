#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResult.h"
#include <memory>
#include <opencv2/opencv.hpp>

class IDebugger;

namespace opencv {

class MaskEngine {
public:
    static api::opencv::MaskResult MaskImage(const api::opencv::MaskRequest& req, std::shared_ptr<IDebugger> debugger = nullptr);
    static cv::Mat MakeLineMask(const cv::Mat& gray, bool horizontal);
};

} // namespace opencv
