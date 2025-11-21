#include "opencv/pch.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "opencv/OpenCvEngine.h"
#include "debug/IDebugger.h"

class OpenCvAdapterImpl : public api::opencv::IOpenCvAdapter {
public:
    OpenCvAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    api::opencv::DenoiseResult denoise(const api::opencv::DenoiseRequest& req) override {
        api::opencv::DenoiseResult res;
        return res;
    }

    api::opencv::MaskResult mask(const api::opencv::MaskRequest& req) override {
        api::opencv::MaskResult res;
        return res;
    }

    api::opencv::DetectResult detect(const api::opencv::DetectRequest& req) override {
        api::opencv::DetectResult res;
        return res;
    }

    api::opencv::CropResult crop(const api::opencv::CropRequest& req) override {
        api::opencv::CropResult res;
        return res;
    }

private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<OpenCvAdapterImpl>(std::move(debugger));
}
