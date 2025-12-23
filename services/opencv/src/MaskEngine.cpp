#include "opencv/pch.h"
#include "opencv/MaskEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace opencv {

cv::Mat MaskEngine::ApplyTextMasks(const cv::Mat& gray, const std::vector<cv::Rect>& rects, int padX, int padY) {
    if (gray.empty()) return gray;
    cv::Mat masked = gray.clone();
    if (masked.channels() > 1) {
        cv::Mat tmp; cv::cvtColor(masked, tmp, cv::COLOR_BGR2GRAY); masked = tmp;
    }
    for (const auto& r : rects) {
        cv::Rect roi(std::max(0, r.x - padX), std::max(0, r.y - padY), r.width + 2 * padX, r.height + 2 * padY);
        roi &= cv::Rect(0, 0, masked.cols, masked.rows);
        if (roi.width > 0 && roi.height > 0) {
            cv::rectangle(masked, roi, cv::Scalar(255), cv::FILLED);
        }
    }
    return masked;
}

api::opencv::MaskResult MaskEngine::MaskImage(const api::opencv::MaskRequest& req, std::shared_ptr<IDebugger> debugger) {
    api::opencv::MaskResult res;
    try {
        const auto path = std::filesystem::path(req.imagePath);
        if (!std::filesystem::exists(path)) return res;

        cv::Mat img = cv::imread(path.string(), cv::IMREAD_COLOR);
        if (img.empty()) return res;

        cv::Mat mask(img.size(), CV_8UC1, cv::Scalar(0));
        for (const auto& e : req.textElements) {
            cv::Rect r(e.x, e.y, e.width, e.height);
            r &= cv::Rect(0, 0, img.cols, img.rows);
            if (r.width <= 0 || r.height <= 0) continue;
            cv::rectangle(mask, r, cv::Scalar(255), cv::FILLED);
        }

        cv::Mat out;
        if (req.mode == api::opencv::MaskRequest::Mode::Whiteout) {
            out = img.clone();
            img.copyTo(out);
            out.setTo(cv::Scalar(255,255,255), mask);
        } else { // Inpaint
            cv::inpaint(img, mask, out, 3.0, cv::INPAINT_TELEA);
        }

        auto stem = path.stem().string();
        auto parent = path.parent_path();
        auto outPath = parent / (stem + "_masked.png");
        if (cv::imwrite(outPath.string(), out)) {
            res.maskedImagePath = outPath;
            if (debugger && debugger->enabled()) {
                try {
                    std::vector<uint8_t> buf; cv::imencode(".png", out, buf);
                    debugger->writeBytes("opencv/" + stem + "/masked.png", buf);
                } catch (...) {}
            }
        }
    } catch (...) {}
    return res;
}

} // namespace opencv
