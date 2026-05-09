/**
 * @file core/src/application/import/ImportStatementStrategy.h
 * @brief Declares private import strategy contracts used only by core import implementation files.
 */

#pragma once

#include "core/application/import/IImportStatement.h"

#include <memory>

namespace core::ports::services {
class IPopplerService;
class IOpenCvService;
class ITesseractService;
}

namespace core::application::importing {

/**
 * @brief Defines the common interface for the default import strategy implementation.
 */
class IImportStatementStrategy {
public:
    /**
     * @brief Destroy the import strategy interface.
     */
    virtual ~IImportStatementStrategy() = default;
    /**
     * @brief Runs the import strategy for a single request.
     * @param req Import execution request.
     * @return Import result produced by the strategy.
     */
    virtual ImportResult run(const ImportRequest& req) = 0;
};

/**
 * @brief Creates the default import strategy implementation over the configured services.
 * @param poppler Poppler adapter.
 * @param opencv OpenCV adapter.
 * @param tesseract Tesseract adapter.
 * @param errorReporter Optional error reporter.
 * @return Default import strategy instance.
 */
std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<core::ports::services::IPopplerService> poppler,
                                                                      std::shared_ptr<core::ports::services::IOpenCvService> opencv,
                                                                      std::shared_ptr<core::ports::services::ITesseractService> tesseract,
                                                                      std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);

} // namespace core::application::importing
