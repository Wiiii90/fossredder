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

std::shared_ptr<IStatementExtractionService> createStatementExtractionService(std::shared_ptr<class IPopplerService> poppler,
                                                                              std::shared_ptr<class IOpenCvService> openCv,
                                                                              std::shared_ptr<class ITesseractService> tesseract);
