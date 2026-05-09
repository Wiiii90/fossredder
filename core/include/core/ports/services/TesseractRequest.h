#pragma once

#include "core/ports/services/TesseractTypes.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace core::ports::services::tesseract {

inline constexpr const char* kDefaultStatementCharWhitelist =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜabcdefghijklmnopqrstuvwxyzäöüß0123456789.,:-/()'“”‘’+&% ";

enum class OcrEngineMode : int {
    LegacyOnly = 0,
    LstmOnly = 1,
    LegacyAndLstm = 2,
    Default = 3
};

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
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    std::vector<Cell> cells;

    std::string tessdataPath;
    RecognitionSettings recognition;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

} // namespace core::ports::services::tesseract

