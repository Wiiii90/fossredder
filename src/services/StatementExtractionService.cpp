#include "pch.h"
#include "services/IStatementExtractionService.h"
#include "services/IExtractionStrategy.h"
#include "services/poppler/IPopplerService.h"
#include "services/opencv/IOpenCvService.h"
#include "services/tesseract/ITesseractService.h"

// Simple service implementation that constructs the default strategy and delegates.
class StatementExtractionServiceImpl : public IStatementExtractionService {
public:
    StatementExtractionServiceImpl(std::shared_ptr<IPopplerService> poppler,
                                   std::shared_ptr<IOpenCvService> openCv,
                                   std::shared_ptr<ITesseractService> tesseract)
        : strategy_(createDefaultExtractionStrategy(std::move(poppler), std::move(openCv), std::move(tesseract))) {}

    StatementExtractionResult extract(const StatementExtractionRequest& req) override {
        if (!strategy_) return {};
        return strategy_->run(req);
    }

private:
    std::unique_ptr<IExtractionStrategy> strategy_;
};

std::shared_ptr<IStatementExtractionService> createStatementExtractionService(std::shared_ptr<IPopplerService> poppler,
                                                                              std::shared_ptr<IOpenCvService> openCv,
                                                                              std::shared_ptr<ITesseractService> tesseract) {
    return std::make_shared<StatementExtractionServiceImpl>(std::move(poppler), std::move(openCv), std::move(tesseract));
}
