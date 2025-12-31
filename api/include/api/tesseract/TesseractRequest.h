#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "api/tesseract/Types.h"

namespace api::tesseract {

struct ExtractRequest {
    enum class Kind { Text, Table } kind = Kind::Text;

    std::filesystem::path imagePath;
    std::vector<Cell> cells;

    std::string tessdataPath;
    int psm = 3;
};

}