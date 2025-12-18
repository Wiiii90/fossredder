#pragma once

#include "api/opencv/Types.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

class IDebugger;

class OpenCvEngine {
public:
    // Detect tables from a grayscale or mask image. Returns vector of api::opencv::Table DTOs.
    static std::vector<api::opencv::Table> detectTablesFromImage(const cv::Mat& img, const std::string& imagePath, int maxTables = 4, std::shared_ptr<IDebugger> debugger = nullptr);

    // Crop given rectangles out of the image and save them to disk. Returns list of saved file paths.
    static std::vector<std::filesystem::path> cropImages(const std::string& imagePath, const std::vector<api::opencv::Rect>& rects, const std::filesystem::path& outputDir = std::filesystem::path(), std::shared_ptr<IDebugger> debugger = nullptr);
};
