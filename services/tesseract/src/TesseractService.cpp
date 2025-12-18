#include "tesseract/pch.h"
#include "api/tesseract/ITesseractService.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResponse.h"
#include <memory>

class TesseractServiceImpl : public api::tesseract::ITesseractService {
public:
    TesseractServiceImpl(std::shared_ptr<api::tesseract::ITesseractAdapter> adapter) : adapter_(std::move(adapter)) {}

    api::tesseract::ExtractResult extract(const api::tesseract::ExtractRequest& req) override {
        if (!adapter_) return api::tesseract::ExtractResult{};
        return adapter_->extract(req);
    }

private:
    std::shared_ptr<api::tesseract::ITesseractAdapter> adapter_;
};

namespace api { namespace tesseract {

std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter) {
    return std::make_shared<TesseractServiceImpl>(std::move(adapter));
}

}} // namespace api::tesseract
