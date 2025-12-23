#include "opencv/pch.h"
#include "opencv/CropEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace opencv {

std::vector<std::filesystem::path> CropEngine::CropImages(const std::string& imagePath, const std::vector<api::opencv::Rect>& rects, const std::filesystem::path& outputDir, std::shared_ptr<IDebugger> debugger) {
    std::vector<std::filesystem::path> outPaths;
    if (rects.empty()) return outPaths;
    if (outputDir.empty()) return outPaths;
    try {
        cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (img.empty()) {
            if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to load image for cropping: " + imagePath);
            return outPaths;
        }

        std::filesystem::create_directories(outputDir);
        std::string stem;
        try { stem = std::filesystem::path(imagePath).stem().string(); } catch (...) { stem = "page"; }

        int idx = 0;
        for (const auto &rct : rects) {
            try {
                cv::Rect r(rct.x, rct.y, rct.width, rct.height);
                r &= cv::Rect(0, 0, img.cols, img.rows);
                if (r.width <= 0 || r.height <= 0) { ++idx; continue; }
                cv::Mat crop = img(r).clone();
                std::ostringstream fname; fname << (outputDir.string()) << "/" << (idx + 1) << ".png";
                std::string filename = fname.str();
                if (!cv::imwrite(filename, crop)) {
                    if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to write crop: " + filename);
                } else {
                    outPaths.emplace_back(filename);
                    if (debugger && debugger->enabled()) {
                        try {
                            std::ifstream ifs(filename, std::ios::binary);
                            if (ifs) {
                                std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                                std::ostringstream dbgRel;
                                dbgRel << "opencv/" << stem << "/" << (idx + 1) << "_cropped";
                                debugger->writeBytes(dbgRel.str() + ".png", buf);
                            }
                        } catch (...) {}
                    }
                }
            } catch (...) {}
            ++idx;
        }
    } catch (...) {}
    return outPaths;
}

} // namespace opencv
