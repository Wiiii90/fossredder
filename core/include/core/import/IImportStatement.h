/**
 * @file core/include/core/import/IImportStatement.h
 * @brief Declares the public statement import service interface and factory.
 */

#pragma once

#include "core/import/ImportRequest.h"
#include "core/import/ImportResult.h"

#include <memory>

namespace api { namespace poppler { class IPopplerService; } }
namespace api { namespace opencv { class IOpenCvService; } }
namespace api { namespace tesseract { class ITesseractService; } }

class IDebugger;

namespace core::importing {

/**
 * @brief Public statement import service used by the import job orchestration.
 */
class IImportStatement {
public:
    virtual ~IImportStatement() = default;
    virtual ImportResult importStatement(const ImportRequest& req) = 0;
};

/**
 * @brief Creates the default statement import service over the configured adapters.
 */
std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                        std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                                        std::shared_ptr<api::tesseract::ITesseractService> tesseract,
                                                        std::shared_ptr<IDebugger> debugger = nullptr);

} // namespace core::importing
