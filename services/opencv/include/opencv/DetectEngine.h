#pragma once

#include "api/opencv/Types.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

class IDebugger;

namespace opencv {

class DetectEngine {
public:
    static std::vector<api::opencv::Table> DetectTables(const cv::Mat& img, const std::string& imagePath, std::shared_ptr<IDebugger> debugger = nullptr);
    static std::vector<cv::Rect> DetectTextBlocks(const cv::Mat& img, std::shared_ptr<IDebugger> debugger = nullptr);
};

} // namespace opencv
