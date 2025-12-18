#include "opencv/pch.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "opencv/OpenCvEngine.h"
#include "debug/IDebugger.h"

#include <opencv2/opencv.hpp>
#include <filesystem>

class OpenCvAdapterImpl : public api::opencv::IOpenCvAdapter {
public:
    OpenCvAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    api::opencv::DenoiseResult denoise(const api::opencv::DenoiseRequest& req) override {
        api::opencv::DenoiseResult res;
        res.denoisedImagePath = req.imagePath;
        return res;
    }

    api::opencv::MaskResult mask(const api::opencv::MaskRequest& req) override {
        api::opencv::MaskResult res;
        res.maskedImagePath = req.imagePath;
        return res;
    }

    api::opencv::DetectResult detect(const api::opencv::DetectRequest& req) override {
        api::opencv::DetectResult res;
        try {
            std::filesystem::path p = req.imagePath;
            if (!std::filesystem::exists(p)) return res;

            cv::Mat img = cv::imread(p.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) return res;

            auto tables = OpenCvEngine::detectTablesFromImage(img, p.string(), 4, debugger);
            if (!tables.empty()) {
                res.table = tables[0];
                res.detected = true;
            }
        } catch (...) {}
        return res;
    }

    api::opencv::CropResult crop(const api::opencv::CropRequest& req) override {
        api::opencv::CropResult res;
        try {
            std::filesystem::path p = req.imagePath;
            if (!std::filesystem::exists(p)) return res;

            // use bbox from req
            std::vector<api::opencv::Rect> rects; rects.push_back(req.bbox);
            auto paths = OpenCvEngine::cropImages(p.string(), rects, std::filesystem::path(), debugger);
            res.croppedImagePaths = std::move(paths);
        } catch (...) {}
        return res;
    }

private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<api::opencv::IOpenCvAdapter> createOpenCvAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<OpenCvAdapterImpl>(std::move(debugger));
}
