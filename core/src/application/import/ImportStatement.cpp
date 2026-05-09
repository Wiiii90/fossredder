/**
 * @file core/src/import/ImportStatement.cpp
 * @brief Factory and thin implementation for IImportStatement using strategies.
 *
 * Provides a small concrete wrapper around an import strategy and a factory
 * function to create a usable IImportStatement instance.
 */

#include "core/pch.h"
#include "core/application/import/IImportStatement.h"
#include "ImportStatementStrategy.h"

#include <memory>

namespace core::application::importing {

namespace {

class ImportStatementImpl final : public IImportStatement {
public:
    explicit ImportStatementImpl(std::unique_ptr<IImportStatementStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    ImportResult importStatement(const ImportRequest& req) override {
        if (!strategy_) return {};
        return strategy_->run(req);
    }

private:
    std::unique_ptr<IImportStatementStrategy> strategy_;
};

} // namespace

std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<core::ports::services::IPopplerService> poppler,
                                                        std::shared_ptr<core::ports::services::IOpenCvService> openCv,
                                                        std::shared_ptr<core::ports::services::ITesseractService> tesseract,
                                                        std::shared_ptr<core::errors::IErrorReporter> errorReporter) {
    auto strat = createDefaultImportStrategy(poppler, openCv, tesseract, std::move(errorReporter));
    return std::make_shared<ImportStatementImpl>(std::move(strat));
}

} // namespace core::application::importing
