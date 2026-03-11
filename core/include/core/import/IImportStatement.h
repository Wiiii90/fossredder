#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <atomic>

#include "core/import/ImportedTransaction.h"

namespace core { namespace jobs { class Scheduler; class SlotLimiter; } }
namespace core::domain { class Statement; }

struct ImportRequest {
    std::string sourcePath;
    std::string runRoot; // absolute directory for all artifacts of this import run
    std::string runIdPrefix; // e.g. yyyyMMddHHmmsszzz (UTC)

    std::string jobId;

    // Optional progress callback: (progress [0..1], phase message)
    std::function<void(double, const std::string&)> progressCallback;

    // Optional shared cancel flag that can be set to true to request cooperative cancellation
    std::shared_ptr<std::atomic<bool>> cancelFlag;

    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;
};

struct ImportResult {
    std::shared_ptr<core::domain::Statement> data;
    std::vector<ImportedTransaction> transactions;
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
