#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace api::poppler {

struct RenderRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    std::filesystem::path outputDir; // optional: where rendered images/metadata will be written
};

struct ExtractRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    int fromPage = 0;
    int toPage = -1;
    std::filesystem::path outputDir; // optional: where metadata will be written
};

}