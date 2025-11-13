#pragma once

#include "services/IStatementExtractionService.h"
#include <memory>

class IPopplerService;
class IOpenCvService;
class ITesseractService;

class IExtractionStrategy {
public:
    virtual ~IExtractionStrategy() = default;
    virtual StatementExtractionResult run(const StatementExtractionRequest& req) = 0;
};

// Factory for default strategy with providers
std::unique_ptr<IExtractionStrategy> createDefaultExtractionStrategy(std::shared_ptr<IPopplerService> poppler,
                                                                    std::shared_ptr<IOpenCvService> layout,
                                                                    std::shared_ptr<ITesseractService> tesseract);
