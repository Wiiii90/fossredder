/**
 * @file core/src/import/parsing/DefaultStatementParser.h
 * @brief Declares the private statement parser entry point used inside import/parser implementation files.
 */

#pragma once

#include "api/opencv/IOpenCvService.h"
#include "api/opencv/Types.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/models/TransactionDraft.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class DefaultStatementParser {
public:
    struct ParseResult {
        std::vector<core::domain::TransactionDraft> transactions;
        std::vector<std::string> debugLines;
        std::unordered_map<std::string, std::vector<uint8_t>> artifacts;
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
