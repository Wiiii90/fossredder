#include "core/pch.h"
#include "core/import/IImportStatement.h"
#include "core/import/IImportStatementStrategy.h"

#include <memory>

class ImportStatementImpl : public IImportStatement {
public:
    ImportStatementImpl(std::unique_ptr<IImportStatementStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    ImportResult importStatement(const ImportRequest& req) override {
        if (!strategy_) return {};
        return strategy_->run(req);
    }

private:
    std::unique_ptr<IImportStatementStrategy> strategy_;
};

std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                        std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                                        std::shared_ptr<api::tesseract::ITesseractService> tesseract,
                                                        std::shared_ptr<IDebugger> debugger) {
    auto strat = createDefaultImportStrategy(poppler, openCv, tesseract, std::move(debugger));
    return std::make_shared<ImportStatementImpl>(std::move(strat));
}
