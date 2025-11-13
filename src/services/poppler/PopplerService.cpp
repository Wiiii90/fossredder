#include "pch.h"
#include "services/poppler/IPopplerAdapter.h"
#include "services/poppler/IPopplerService.h"
#include <memory>

class PopplerServiceImpl : public IPopplerService {
public:
    PopplerServiceImpl(std::shared_ptr<IPopplerAdapter> adapter) : adapter_(std::move(adapter)) {}

    RenderResult render(const RenderRequest& req) override {
        return adapter_->renderDocument(req);
    }

    ExtractResult extract(const ExtractRequest& req) override {
        return adapter_->extractDocumentMeta(req);
    }

private:
    std::shared_ptr<IPopplerAdapter> adapter_;
};

std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter) {
    return std::make_shared<PopplerServiceImpl>(std::move(adapter));
}
