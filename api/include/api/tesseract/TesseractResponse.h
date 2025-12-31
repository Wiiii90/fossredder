#pragma once

#include <string>
#include <vector>
#include "api/tesseract/Types.h"

namespace api::tesseract {

struct ExtractResult {
    std::string text;
    std::string tsv;
    std::vector<Word> words;
    std::vector<Table> tables;
};

}