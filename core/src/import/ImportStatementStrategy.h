/**
 * @file core/src/import/ImportStatementStrategy.h
 * @brief Declares private import strategy contracts used only by core import implementation files.
 */

#pragma once

#include "core/import/IImportStatement.h"

#include <memory>

namespace api { namespace poppler { class IPopplerService; } }
namespace api { namespace opencv { class IOpenCvService; } }
namespace api { namespace tesseract { class ITesseractService; } }
class IDebugger;

namespace core::importing {

class IImportStatementStrategy {
public:
    virtual ~IImportStatementStrategy() = default;
    virtual ImportResult run(const ImportRequest& req) = 0;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                                      std::shared_ptr<api::opencv::IOpenCvService> opencv,
                                                                      std::shared_ptr<api::tesseract::ITesseractService> tesseract,
                                                                      std::shared_ptr<IDebugger> debugger = nullptr);

} // namespace core::importing
