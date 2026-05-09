#include "poppler/pch.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/IPopplerService.h"
#include <memory>
#include <utility>

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

namespace api { namespace poppler {

std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter) {
    return std::make_shared<PopplerServiceImpl>(std::move(adapter));
}

}} // namespace api::poppler
