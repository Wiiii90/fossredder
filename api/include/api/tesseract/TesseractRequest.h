#pragma once

#include <filesystem>
#include <string>

namespace api::tesseract {

struct ExtractRequest {
    std::filesystem::path imagePath;
    std::string tessdataPath;
    int psm = 3;
};

}