/**
 * @file core/include/core/ports/text-recognition/TesseractResult.h
 * @brief Result types for text recognition operations.
 */
#pragma once

#include "core/ports/text-recognition/TesseractTypes.h"

#include <string>
#include <vector>

namespace core::ports::text_recognition::tesseract {

/**
 * @brief OCR extraction result.
 */
struct ExtractResult {
    std::string text;
    std::string tsv;
    std::vector<Word> words;
    std::vector<Table> tables;
};

} // namespace core::ports::text_recognition::tesseract
