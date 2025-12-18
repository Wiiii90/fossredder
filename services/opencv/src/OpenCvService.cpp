#include "opencv/pch.h"
#include "api/opencv/IOpenCvService.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include <memory>

class OpenCvServiceImpl : public api::opencv::IOpenCvService {
public:
    OpenCvServiceImpl(std::shared_ptr<api::opencv::IOpenCvAdapter> adapter) : adapter_(std::move(adapter)) {}

    api::opencv::DenoiseResult denoise(const api::opencv::DenoiseRequest& req) const override {
        if (!adapter_) return api::opencv::DenoiseResult{};
        return adapter_->denoise(req);
    }

    api::opencv::MaskResult mask(const api::opencv::MaskRequest& req) const override {
        if (!adapter_) return api::opencv::MaskResult{};
        return adapter_->mask(req);
    }

    api::opencv::DetectResult detect(const api::opencv::DetectRequest& req) const override {
        if (!adapter_) return api::opencv::DetectResult{};
        return adapter_->detect(req);
    }

    api::opencv::CropResult crop(const api::opencv::CropRequest& req) const override {
        if (!adapter_) return api::opencv::CropResult{};
        return adapter_->crop(req);
    }

private:
    std::shared_ptr<api::opencv::IOpenCvAdapter> adapter_;
};

namespace api { namespace opencv {

std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter) {
    return std::make_shared<OpenCvServiceImpl>(std::move(adapter));
}

}} // namespace api::opencv
