#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace api::poppler {

struct RenderRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    std::filesystem::path outputDir; // required: where rendered images/metadata will be written
    std::string uniqIdPrefix;        // unique id per request
    std::string filePrefix;          // e.g. "poppler_render"
};

struct ExtractRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    int fromPage = 0;
    int toPage = -1;
    std::filesystem::path outputDir; // required: where metadata will be written
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "poppler_extract"
};

}