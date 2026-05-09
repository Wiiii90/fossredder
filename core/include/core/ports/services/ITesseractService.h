#pragma once

#include "core/ports/services/TesseractRequest.h"
#include "core/ports/services/TesseractResult.h"

namespace core::ports::services {

class ITesseractService {
public:
    virtual ~ITesseractService() = default;

    virtual tesseract::ExtractResult extract(const tesseract::ExtractRequest& req) = 0;
};

} // namespace core::ports::services
