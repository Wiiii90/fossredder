#pragma once

#include <string>
#include <vector>

#include "api/opencv/Types.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/models/Transaction.h"

class DefaultStatementParser {
public:
    struct ParseResult {
        std::vector<Transaction> transactions;
        std::vector<std::string> debugLines;
        std::string lastBookingDate;
    };

    static ParseResult parse(const api::opencv::Table& table, const api::tesseract::ExtractResult& ocr, std::string initialBookingDate = {});
};
