#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include <memory>
#include <filesystem>
#include <opencv2/opencv.hpp>

class IDebugger;

namespace opencv {

class CropEngine {
public:
    static std::vector<std::filesystem::path> CropImages(const std::string& imagePath,
                                                        const std::vector<api::opencv::Rect>& rects,
                                                        const std::filesystem::path& outputDir,
                                                        api::opencv::CropRequest::OutputFormat fmt,
                                                        int jpegQuality = 92,
                                                        std::shared_ptr<IDebugger> debugger = nullptr,
                                                        const std::string& filePrefix = "");

    static std::vector<std::filesystem::path> CropImages(const cv::Mat& img,
                                                        const std::vector<api::opencv::Rect>& rects,
                                                        const std::filesystem::path& outputDir,
                                                        api::opencv::CropRequest::OutputFormat fmt,
                                                        int jpegQuality = 92,
                                                        std::vector<std::vector<uint8_t>>* outBytes = nullptr,
                                                        std::shared_ptr<IDebugger> debugger = nullptr,
                                                        const std::string& filePrefix = "");
};

} // namespace opencv
