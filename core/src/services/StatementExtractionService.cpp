#include "core/pch.h"
#include "core/services/IStatementExtractionService.h"
#include "core/services/IExtractionStrategy.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/ITesseractService.h"

class StatementExtractionServiceImpl : public IStatementExtractionService {
public:
    StatementExtractionServiceImpl(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                   std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                   std::shared_ptr<api::tesseract::ITesseractService> tesseract)
        : strategy_(createDefaultExtractionStrategy(std::move(poppler), std::move(openCv), std::move(tesseract))) {}

    StatementExtractionResult extract(const StatementExtractionRequest& req) override {
        if (!strategy_) return {};
        return strategy_->run(req);
    }

private:
    std::unique_ptr<IExtractionStrategy> strategy_;
};

std::shared_ptr<IStatementExtractionService> createStatementExtractionService(std::shared_ptr<api::poppler::IPopplerService> poppler,
                                                                              std::shared_ptr<api::opencv::IOpenCvService> openCv,
                                                                              std::shared_ptr<api::tesseract::ITesseractService> tesseract) {
    return std::make_shared<StatementExtractionServiceImpl>(std::move(poppler), std::move(openCv), std::move(tesseract));
}
