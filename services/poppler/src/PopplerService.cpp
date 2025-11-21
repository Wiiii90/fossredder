#include "poppler/pch.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/IPopplerService.h"
#include <memory>

class PopplerServiceImpl : public api::poppler::IPopplerService {
public:
    PopplerServiceImpl(std::shared_ptr<api::poppler::IPopplerAdapter> adapter) : adapter_(std::move(adapter)) {}

    api::poppler::RenderResult render(const api::poppler::RenderRequest& req) override {
        return adapter_->render(req);
    }

    api::poppler::ExtractResult extract(const api::poppler::ExtractRequest& req) override {
        return adapter_->extract(req);
    }

private:
    std::shared_ptr<api::poppler::IPopplerAdapter> adapter_;
};

std::shared_ptr<api::poppler::IPopplerService> createPopplerService(std::shared_ptr<api::poppler::IPopplerAdapter> adapter) {
    return std::make_shared<PopplerServiceImpl>(std::move(adapter));
}
