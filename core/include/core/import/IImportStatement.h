/**
 * @file core/include/core/import/IImportStatement.h
 * @brief Declares the public statement import service interface and factory.
 */

#pragma once

#include "core/import/ImportRequest.h"
#include "core/import/ImportResult.h"

#include <memory>

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
