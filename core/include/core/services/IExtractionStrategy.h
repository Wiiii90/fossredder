#pragma once

#include "core/services/IStatementExtractionService.h"
#include <memory>

namespace api {
    namespace poppler { class IPopplerService; }
    namespace opencv { class IOpenCvService; }
    namespace tesseract { class ITesseractService; }
}

class IExtractionStrategy {
public:
    virtual ~IExtractionStrategy() = default;
    virtual StatementExtractionResult run(const StatementExtractionRequest& req) = 0;
};

// Factory for default strategy with providers
std::unique_ptr<IExtractionStrategy> createDefaultExtractionStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                                    std::shared_ptr<api::opencv::IOpenCvService> opencv,
                                                                    std::shared_ptr<api::tesseract::ITesseractService> tesseract);
