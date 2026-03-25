#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <atomic>
#include "api/tesseract/Types.h"

namespace api::tesseract {

inline constexpr const char* kDefaultStatementCharWhitelist =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜabcdefghijklmnopqrstuvwxyzäöüß0123456789.,:-/()'“”‘’+&% ";

enum class OcrEngineMode : int {
    LegacyOnly = 0,
    LstmOnly = 1,
    LegacyAndLstm = 2,
    Default = 3
};

/** @brief User-facing OCR tuning that can be forwarded through the import pipeline. */
struct RecognitionSettings {
    std::string language = "deu";
    OcrEngineMode engineMode = OcrEngineMode::LstmOnly;
    bool preserveInterwordSpaces = true;
    std::string charWhitelist = kDefaultStatementCharWhitelist;
    int psm = -1;
};

struct ExtractRequest {
    enum class Kind { Text, Table } kind = Kind::Text;

    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir; // optional
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "tesseract_extract_table_page1"
    std::vector<Cell> cells;

    std::string tessdataPath;
    RecognitionSettings recognition;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

}