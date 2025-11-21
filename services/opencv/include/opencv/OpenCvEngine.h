#pragma once

#include "OpenCvDTO.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class OpenCvEngine {
public:
    // Detect tables from a grayscale or mask image. Returns vector of Table DTOs.
    static std::vector<Table> detectTablesFromImage(const cv::Mat& img, const std::string& imagePath, int maxTables = 4, std::shared_ptr<IDebugger> debugger = nullptr);
};
