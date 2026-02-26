#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace api::poppler {

struct RenderRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    std::filesystem::path outputDir; // optional: where rendered images/metadata may be written
    std::string uniqIdPrefix;        // unique id per request
    std::string filePrefix;          // e.g. "poppler_render"

    std::shared_ptr<std::atomic<bool>> cancelFlag; // optional cooperative cancel flag
};

struct ExtractRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    int fromPage = 0;
    int toPage = -1;
    std::filesystem::path outputDir; // optional: where metadata may be written
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "poppler_extract"

    std::shared_ptr<std::atomic<bool>> cancelFlag; // optional cooperative cancel flag
};

}