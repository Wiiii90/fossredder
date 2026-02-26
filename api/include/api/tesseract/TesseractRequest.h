#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <atomic>
#include "api/tesseract/Types.h"

namespace api::tesseract {

struct ExtractRequest {
    enum class Kind { Text, Table } kind = Kind::Text;

    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir; // optional
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "tesseract_extract_table_page1"
    std::vector<Cell> cells;

    std::string tessdataPath;
    int psm = 3;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

}