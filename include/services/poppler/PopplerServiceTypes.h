#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct RenderRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
};

struct RenderResult {
    std::vector<std::filesystem::path> images;
};

struct ExtractRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    int fromPage = 0;
    int toPage = -1;
};

struct ExtractResult {
    nlohmann::json metadata;
};
