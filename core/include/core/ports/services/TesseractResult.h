#pragma once

#include "core/ports/services/TesseractTypes.h"

#include <string>
#include <vector>

namespace core::ports::services::tesseract {

struct ExtractResult {
    std::string text;
    std::string tsv;
    std::vector<Word> words;
    std::vector<Table> tables;
};

} // namespace core::ports::services::tesseract

