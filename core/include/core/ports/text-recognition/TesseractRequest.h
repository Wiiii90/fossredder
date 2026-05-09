/**
 * @file core/include/core/ports/text-recognition/TesseractRequest.h
 * @brief Request types for text recognition operations.
 */
#pragma once

#include "core/ports/text-recognition/TesseractTypes.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace core::ports::text_recognition::tesseract {

/**
 * @brief Recognition settings for OCR.
 */
struct RecognitionSettings {
    std::string language = "deu";
    OcrEngineMode engineMode = OcrEngineMode::LstmOnly;
    bool preserveInterwordSpaces = true;
    std::string charWhitelist = kDefaultStatementCharWhitelist;
    int psm = -1;
};

/**
 * @brief Request for OCR extraction.
 */
struct ExtractRequest {
    enum class Kind { Text, Table } kind = Kind::Text;

    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    std::vector<Cell> cells;

    std::string tessdataPath;
    RecognitionSettings recognition;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

} // namespace core::ports::text_recognition::tesseract
