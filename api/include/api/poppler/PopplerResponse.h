#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace api::poppler {

struct RenderResult {
    std::vector<std::filesystem::path> images;
};

struct ExtractResult {
    nlohmann::json metadata;
};

}