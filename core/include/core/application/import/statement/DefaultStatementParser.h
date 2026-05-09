/**
 * @file core/include/core/application/import/statement/DefaultStatementParser.h
 * @brief Declares the public default statement parser used by the import pipeline.
 */

#pragma once

#include "api/opencv/OpenCvResult.h"
#include "api/tesseract/TesseractResult.h"
#include "core/ports/services/IOpenCvService.h"
#include "core/application/import/draft/TransactionDraft.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::application::importing::statement {

class DefaultStatementParser {
public:
    /**
     * @brief Holds the parser output for one imported statement page.
     */
    struct ParseResult {
        std::vector<core::application::importing::draft::TransactionDraft> transactions;
        std::vector<std::string> debugLines;
        std::unordered_map<std::string, std::vector<uint8_t>> artifacts;
        std::string lastBookingDate;
        int nextTransactionIndex = 1;
    };

    /**
     * @brief Parses an OCR-backed table into transaction drafts and artifacts.
     * @param table Detected table structure from the page image.
     * @param ocr OCR result for the page or table region.
     * @param pageCropImagePath File path to the cropped page image.
     * @param opencv OpenCV service used for image extraction helpers.
     * @param pageCropImageBytes Raw bytes of the cropped page image.
     * @param initialBookingDate Booking date seed carried into parsing.
     * @param initialTransactionIndex Initial transaction counter value.
     * @return Parse result containing generated drafts, artifacts, and parser metadata.
     */
    static ParseResult parse(const api::opencv::Table& table,
                             const api::tesseract::ExtractResult& ocr,
                             const std::string& pageCropImagePath,
                             std::shared_ptr<core::ports::services::IOpenCvService> opencv,
                             const std::vector<uint8_t>& pageCropImageBytes,
                             std::string initialBookingDate = {},
                             int initialTransactionIndex = 1);
};

} // namespace core::application::importing::statement

namespace core {
using DefaultStatementParser = application::importing::statement::DefaultStatementParser;
} // namespace core
