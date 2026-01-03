#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

class Statement;

struct ImportRequest {
    std::string sourcePath;
    std::string runRoot; // absolute directory for all artifacts of this import run
    std::string runIdPrefix; // e.g. yyyyMMddHHmmsszzz (UTC)
};

struct ImportResult {
    std::shared_ptr<Statement> data;
    std::map<std::string, std::vector<uint8_t>> artifacts;
};

class IImportStatement {
public:
    virtual ~IImportStatement() = default;
    virtual ImportResult importStatement(const ImportRequest& req) = 0;
};

namespace api { namespace poppler { class IPopplerService; } }
namespace api { namespace opencv { class IOpenCvService; } }
namespace api { namespace tesseract { class ITesseractService; } }

class IDebugger;

std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                        std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                                        std::shared_ptr<api::tesseract::ITesseractService> tesseract,
                                                        std::shared_ptr<IDebugger> debugger = nullptr);
