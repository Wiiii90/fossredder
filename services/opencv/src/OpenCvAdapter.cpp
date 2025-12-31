#include "opencv/pch.h"
#include "api/opencv/IOpenCvAdapter.h"
#include "opencv/DenoiseEngine.h"
#include "opencv/MaskEngine.h"
#include "opencv/DetectEngine.h"
#include "opencv/CropEngine.h"
#include "debug/IDebugger.h"

#include <opencv2/opencv.hpp>
#include <filesystem>

namespace {
std::filesystem::path resolveCropOutputDir(const std::filesystem::path& outputDir, const std::string& stem) {
    if (!outputDir.empty()) return outputDir;
    try {
        auto tmp = std::filesystem::temp_directory_path() / "fossredder" / "opencv" / "output" / stem;
        std::filesystem::create_directories(tmp);
        return tmp;
    } catch (...) {
        return std::filesystem::current_path();
    }
}
}

class OpenCvAdapterImpl : public api::opencv::IOpenCvAdapter {
public:
    OpenCvAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    api::opencv::DenoiseResult denoise(const api::opencv::DenoiseRequest& req) override {
        return opencv::DenoiseEngine::Denoise(req, debugger);
    }

    api::opencv::MaskResult mask(const api::opencv::MaskRequest& req) override {
        return opencv::MaskEngine::MaskImage(req, debugger);
    }

    api::opencv::DetectResult detect(const api::opencv::DetectRequest& req) override {
        api::opencv::DetectResult res;
        try {
            std::filesystem::path p = req.imagePath;
            if (!std::filesystem::exists(p)) return res;

            cv::Mat img = cv::imread(p.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) return res;

            if (req.kind == api::opencv::DetectRequest::DetectKind::TextBlocks) {
                auto blocks = opencv::DetectEngine::DetectTextBlocks(img, debugger);
                if (!blocks.empty()) {
                    res.detected = true;
                    res.textBlocks.reserve(blocks.size());
                    for (const auto &b : blocks) {
                        api::opencv::Rect r;
                        r.x = b.x; r.y = b.y; r.width = b.width; r.height = b.height;
                        res.textBlocks.push_back(r);
                    }
                }
                return res;
            }

            auto tables = opencv::DetectEngine::DetectTables(img, p.string(), debugger);
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

            std::string stem;
            try { stem = p.stem().string(); } catch (...) { stem = "page"; }
            auto outDir = resolveCropOutputDir(std::filesystem::path(), stem);
            if (outDir.empty()) return res;

            std::vector<api::opencv::Rect> rects; rects.push_back(req.bbox);
            auto paths = opencv::CropEngine::CropImages(p.string(), rects, outDir, debugger);
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
