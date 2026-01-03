#include "opencv/pch.h"
#include "opencv/MaskEngine.h"
#include "opencv/DetectEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <sstream>

namespace opencv {

cv::Mat MaskEngine::MakeLineMask(const cv::Mat& grayIn, bool horizontal) {
    if (grayIn.empty()) return grayIn;
    cv::Mat gray;
    if (grayIn.type() == CV_8UC1) gray = grayIn;
    else if (grayIn.channels() == 3) cv::cvtColor(grayIn, gray, cv::COLOR_BGR2GRAY);
    else grayIn.convertTo(gray, CV_8U);

    cv::Mat bin;
    try {
        cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 31, 10);
    } catch (...) {
        cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    }

    const int span = horizontal ? std::max(40, gray.cols / 15) : std::max(40, gray.rows / 15);
    const int thickness = 2;

    cv::Mat k = horizontal
        ? cv::getStructuringElement(cv::MORPH_RECT, cv::Size(span, thickness))
        : cv::getStructuringElement(cv::MORPH_RECT, cv::Size(thickness, span));

    cv::Mat m;
    try { cv::erode(bin, m, k); } catch (...) { m = bin.clone(); }
    try { cv::dilate(m, m, k); } catch (...) {}

    try {
        cv::Mat closeK = horizontal
            ? cv::getStructuringElement(cv::MORPH_RECT, cv::Size(std::max(5, gray.cols / 120), 1))
            : cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, std::max(5, gray.rows / 120)));
        cv::morphologyEx(m, m, cv::MORPH_CLOSE, closeK);
    } catch (...) {}

    return m;
}

api::opencv::MaskResult MaskEngine::MaskImage(const api::opencv::MaskRequest& req, std::shared_ptr<IDebugger> debugger) {
    api::opencv::MaskResult res;
    try {
        const auto path = std::filesystem::path(req.imagePath);
        if (!std::filesystem::exists(path)) return res;
        if (req.outputDir.empty()) return res;

        cv::Mat img = cv::imread(path.string(), cv::IMREAD_COLOR);
        if (img.empty()) return res;

        const double imgArea = static_cast<double>(img.cols) * static_cast<double>(img.rows);

        std::vector<cv::Rect> boxes;
        boxes.reserve(req.textElements.size());
        for (const auto& e : req.textElements) boxes.emplace_back(e.x, e.y, e.width, e.height);

        if (req.useMorphology) {
            cv::Mat gray; cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            auto blocks = opencv::DetectEngine::DetectTextBlocks(gray, debugger);
            for (const auto &b : blocks) {
                double area = static_cast<double>(b.width) * static_cast<double>(b.height);
                if (area <= imgArea * 0.15) boxes.emplace_back(b.x, b.y, b.width, b.height);
            }
        }

        if (!req.tesseractTsv.empty()) {
            std::istringstream iss(req.tesseractTsv);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.empty()) continue;
                std::istringstream ls(line);
                std::string text; int left, top, width, height; double conf;
                if (!(ls >> text >> left >> top >> width >> height >> conf)) continue;
                double area = static_cast<double>(width) * static_cast<double>(height);
                if (area <= imgArea * 0.25) boxes.emplace_back(left, top, width, height);
            }
        }

        cv::Mat mask(img.size(), CV_8UC1, cv::Scalar(0));

        int padPx = std::max(6, img.cols / 400);
        for (const auto &r : boxes) {
            cv::Rect rr(r.x - padPx, r.y - padPx, r.width + 2 * padPx, r.height + 2 * padPx);
            rr &= cv::Rect(0,0,img.cols,img.rows);
            if (rr.width > 0 && rr.height > 0) cv::rectangle(mask, rr, cv::Scalar(255), cv::FILLED);
        }

        cv::Mat cleaned;
        int dilateK = std::max(3, padPx / 2);
        try { cv::dilate(mask, cleaned, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(dilateK, dilateK))); } catch(...) { cleaned = mask.clone(); }

        int hKernel = std::max(25, img.cols / 100);
        try { cv::morphologyEx(cleaned, cleaned, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(hKernel, 3))); } catch(...) {}
        try { cv::morphologyEx(cleaned, cleaned, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5))); } catch(...) {}
        try { cv::medianBlur(cleaned, cleaned, 3); } catch(...) {}

        try { cv::threshold(cleaned, cleaned, 128, 255, cv::THRESH_BINARY); } catch(...) {}

        double nonZero = static_cast<double>(cv::countNonZero(cleaned));
        if (nonZero > imgArea * 0.4) cleaned = mask;

        cv::Mat out;
        const bool isWhiteout = (req.mode == api::opencv::MaskRequest::Mode::Whiteout);
        if (isWhiteout) {
            out = img.clone();
            out.setTo(cv::Scalar(255,255,255), cleaned);
        } else {
            cv::inpaint(img, cleaned, out, 3.0, cv::INPAINT_TELEA);
        }

        std::filesystem::create_directories(req.outputDir);

        std::string base;
        if (!req.uniqIdPrefix.empty()) {
            base = req.uniqIdPrefix;
            if (!req.filePrefix.empty()) base += "_" + req.filePrefix;
        } else {
            base = req.filePrefix.empty() ? std::string("opencv_mask") : req.filePrefix;
        }

        // Only persist the masked output used by the pipeline.
        const auto outPath = std::filesystem::path(req.outputDir) / (base + ".png");

        if (cv::imwrite(outPath.string(), out)) {
            res.maskedImagePath = outPath;
            if (debugger && debugger->enabled()) {
                try {
                    // Debug-only artifacts: persist to debug_output via debugger (not into runRoot).
                    try {
                        std::vector<uint8_t> mbuf; cv::imencode(".png", cleaned, mbuf);
                        debugger->writeBytes(std::string("opencv_mask_binary"), mbuf);
                    } catch (...) {}

                    std::vector<uint8_t> buf; cv::imencode(".png", out, buf);
                    debugger->writeBytes(isWhiteout ? std::string("opencv_mask_whiteout") : std::string("opencv_mask_inpaint"), buf);
                } catch (...) {}
            }
        }
    } catch (...) {}
    return res;
}

} // namespace opencv
