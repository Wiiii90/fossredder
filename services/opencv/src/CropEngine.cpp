#include "opencv/pch.h"
#include "opencv/CropEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace opencv {

std::vector<std::filesystem::path> CropEngine::CropImages(const std::string& imagePath,
                                                         const std::vector<api::opencv::Rect>& rects,
                                                         const std::filesystem::path& outputDir,
                                                         api::opencv::CropRequest::OutputFormat fmt,
                                                         int jpegQuality,
                                                         std::shared_ptr<IDebugger> debugger,
                                                         const std::string& filePrefix) {
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

        int idx = 0;
        for (const auto &rct : rects) {
            try {
                cv::Rect r(rct.x, rct.y, rct.width, rct.height);
                r &= cv::Rect(0, 0, img.cols, img.rows);
                if (r.width <= 0 || r.height <= 0) { ++idx; continue; }
                cv::Mat crop = img(r).clone();
                const char* ext = (fmt == api::opencv::CropRequest::OutputFormat::Jpg) ? ".jpg" : ".png";
                std::ostringstream fname;
                fname << (outputDir.string()) << "/";
                if (!filePrefix.empty()) fname << filePrefix << "_";
                fname << (idx + 1) << ext;
                std::string filename = fname.str();
                std::vector<int> params;
                if (fmt == api::opencv::CropRequest::OutputFormat::Jpg) {
                    params.push_back(cv::IMWRITE_JPEG_QUALITY);
                    params.push_back(std::max(1, std::min(100, jpegQuality)));
                }

                if (!cv::imwrite(filename, crop, params)) {
                    if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to write crop: " + filename);
                } else {
                    outPaths.emplace_back(filename);
                    if (debugger && debugger->enabled()) {
                        try {
                            std::ifstream ifs(filename, std::ios::binary);
                            if (ifs) {
                                std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                                debugger->writeBytes(std::string("opencv_cropped"), buf);
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
