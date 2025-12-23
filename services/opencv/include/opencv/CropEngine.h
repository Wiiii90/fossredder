#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include <memory>
#include <filesystem>

class IDebugger;

namespace opencv {

class CropEngine {
public:
    static std::vector<std::filesystem::path> CropImages(const std::string& imagePath, const std::vector<api::opencv::Rect>& rects, const std::filesystem::path& outputDir, std::shared_ptr<IDebugger> debugger = nullptr);
};

} // namespace opencv
