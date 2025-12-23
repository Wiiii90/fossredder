#include "opencv/pch.h"
#include "opencv/DenoiseEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace opencv {

namespace {
cv::Mat applyDenoise(const cv::Mat& img, api::opencv::DenoiseRequest::Method method) {
    if (img.empty()) return cv::Mat();
    cv::Mat out;
    switch (method) {
    case api::opencv::DenoiseRequest::Method::Median:
        cv::medianBlur(img, out, 3);
        break;
    case api::opencv::DenoiseRequest::Method::Gaussian:
        cv::GaussianBlur(img, out, cv::Size(3, 3), 0);
        break;
    case api::opencv::DenoiseRequest::Method::Bilateral:
        cv::bilateralFilter(img, out, 5, 50, 50);
        break;
    }
    return out.empty() ? img : out;
}
}

api::opencv::DenoiseResult DenoiseEngine::Denoise(const api::opencv::DenoiseRequest& req, std::shared_ptr<IDebugger> debugger) {
    api::opencv::DenoiseResult res;
    try {
        const auto path = std::filesystem::path(req.imagePath);
        if (!std::filesystem::exists(path)) return res;

        cv::Mat src = cv::imread(path.string(), cv::IMREAD_COLOR);
        if (src.empty()) return res;
        cv::Mat denoised = applyDenoise(src, req.method);

        auto stem = path.stem().string();
        auto parent = path.parent_path();
        auto outPath = parent / (stem + "_denoised.png");
        if (cv::imwrite(outPath.string(), denoised)) {
            res.denoisedImagePath = outPath;
            if (debugger && debugger->enabled()) {
                try {
                    std::vector<uint8_t> buf;
                    cv::imencode(".png", denoised, buf);
                    debugger->writeBytes("opencv/" + stem + "/denoised.png", buf);
                } catch (...) {}
            }
        }
    } catch (...) {}
    return res;
}

} // namespace opencv
