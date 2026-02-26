#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include "api/opencv/Types.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/models/Transaction.h"

class DefaultStatementParser {
public:
    struct ParseResult {
        std::vector<std::shared_ptr<Transaction>> transactions;
        std::vector<std::string> debugLines;
        std::string lastBookingDate;
        int nextTransactionIndex = 1;
    };

    static ParseResult parse(const api::opencv::Table& table,
                            const api::tesseract::ExtractResult& ocr,
                            const std::string& pageCropImagePath,
                            std::shared_ptr<api::opencv::IOpenCvService> opencv,
                            const std::vector<uint8_t>& pageCropImageBytes,
                            const std::filesystem::path& proofOutputDir,
                            std::string initialBookingDate = {},
                            int initialTransactionIndex = 1);
};
