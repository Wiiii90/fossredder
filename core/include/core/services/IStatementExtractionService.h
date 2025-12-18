#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

class StatementData;

struct StatementExtractionRequest {
    std::string sourcePath;
};

struct StatementExtractionResult {
    std::shared_ptr<StatementData> data;
    std::map<std::string, std::vector<uint8_t>> artifacts;
};

class IStatementExtractionService {
public:
    virtual ~IStatementExtractionService() = default;
    virtual StatementExtractionResult extract(const StatementExtractionRequest& req) = 0;
};

// Forward-declare API service interfaces in their namespaces to avoid header dependency
namespace api {
namespace poppler { class IPopplerService; }
namespace opencv { class IOpenCvService; }
namespace tesseract { class ITesseractService; }
}

std::shared_ptr<IStatementExtractionService> createStatementExtractionService(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                                              std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                                                              std::shared_ptr<api::tesseract::ITesseractService> tesseract);
