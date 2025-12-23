#include "opencv/pch.h"
#include "opencv/DetectEngine.h"
#include "opencv/MaskEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace opencv {
namespace {

void writeImageViaDebugger(std::shared_ptr<IDebugger> debugger, const std::string& relPath, const cv::Mat& img) {
    if (!debugger) return;
    std::vector<uint8_t> buf;
    try {
        cv::Mat out;
        if (img.empty()) return;
        if (img.type() == CV_8UC1) {
            cv::cvtColor(img, out, cv::COLOR_GRAY2BGR);
        } else if (img.type() == CV_8UC3) {
            out = img;
        } else {
            img.convertTo(out, CV_8U, 255.0);
            if (out.channels() == 1) cv::cvtColor(out, out, cv::COLOR_GRAY2BGR);
        }
        bool ok = cv::imencode(".png", out, buf);
        if (!ok || buf.empty()) return;
        debugger->writeBytes(relPath + ".png", buf);
    } catch (...) {}
}

std::vector<cv::Rect> findTextBlocksUsingMorphology(const cv::Mat& gray, const std::string& imageStem = std::string(), std::shared_ptr<IDebugger> debugger = nullptr) {
    cv::Mat bin;
    cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 15, 10);
    cv::Mat morph;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 3));
    cv::morphologyEx(bin, morph, cv::MORPH_CLOSE, kernel);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> blocks;
    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        if (rect.width > 50 && rect.height > 10) {
            try {
                cv::Rect rcl = rect & cv::Rect(0,0,bin.cols, bin.rows);
                int white = cv::countNonZero(bin(rcl));
                double whiteRatio = double(white) / double(rcl.area() + 1);
                if (whiteRatio > 0.6) continue;
            } catch (...) {}
            blocks.push_back(rect);
        }
    }
    return blocks;
}

std::vector<int> clusterAndAverageCoordinates(const std::vector<int>& coords, int tolerance) {
    std::vector<int> out;
    if (coords.empty()) return out;
    std::vector<int> s = coords;
    std::sort(s.begin(), s.end());
    int sum = s[0]; int cnt = 1;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] - s[i - 1] <= tolerance) { sum += s[i]; ++cnt; }
        else { out.push_back(sum / cnt); sum = s[i]; cnt = 1; }
    }
    out.push_back(sum / cnt);
    return out;
}

api::opencv::Table detectTableGridHough(const cv::Mat& roiGray, const cv::Rect& roiOffset, const std::vector<cv::Rect>& pageTextBoxes, int& outCells, double& outQuality, const std::string& imageStem, std::shared_ptr<IDebugger> debugger) {
    api::opencv::Table table;
    outCells = 0; outQuality = 0.0;
    if (roiGray.empty()) return table;

    std::vector<cv::Rect> overlappingBoxes;
    for (const auto& tb : pageTextBoxes) {
        cv::Rect rel = tb & cv::Rect(roiOffset.x, roiOffset.y, roiOffset.width, roiOffset.height);
        if (rel.width > 0 && rel.height > 0) {
            cv::Rect r(rel.x - roiOffset.x, rel.y - roiOffset.y, rel.width, rel.height);
            overlappingBoxes.push_back(r);
        }
    }

    int padX = std::max(2, std::max(1, roiGray.cols / 100));
    int padY = std::max(1, std::max(1, roiGray.rows / 100));
    cv::Mat work = MaskEngine::ApplyTextMasks(roiGray, overlappingBoxes, padX, padY);

    if (debugger && debugger->enabled()) {
        try {
            std::ostringstream base; base << "opencv/" << imageStem << "/roi_" << roiOffset.x << "_" << roiOffset.y;
            writeImageViaDebugger(debugger, base.str() + "_work_masked", work);
            try {
                std::ostringstream info;
                info << "roiOffset=" << roiOffset.x << "," << roiOffset.y << "," << roiOffset.width << "x" << roiOffset.height << "\n";
                info << "pageTextBoxes:\n";
                for (const auto &tb : pageTextBoxes) info << tb.x << "," << tb.y << "," << tb.width << "x" << tb.height << "\n";
                info << "overlappingBoxes (roi-local):\n";
                for (const auto &ob : overlappingBoxes) info << ob.x << "," << ob.y << "," << ob.width << "x" << ob.height << "\n";
                debugger->writeText(base.str() + "_debug_boxes.txt", info.str());
            } catch (...) {}
        } catch (...) {}
    }

    cv::Mat edges;
    cv::Canny(work, edges, 50, 150);
    double edgeDensity = static_cast<double>(cv::countNonZero(edges)) / static_cast<double>(roiGray.total());

    int minLenH = std::max(roiGray.cols / 6, 20);
    int minLenV = std::max(roiGray.rows / 6, 20);
    int thresh = std::max(50, std::min(roiGray.cols / 30, roiGray.rows / 30));

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, thresh, std::min(minLenH, minLenV), std::max(1, std::min(roiGray.cols, roiGray.rows) / 200));

    std::vector<int> horizYs, vertXs;
    for (auto& l : lines) {
        int x1 = l[0], y1 = l[1], x2 = l[2], y2 = l[3];
        double angle = std::atan2((double)(y2 - y1), (double)(x2 - x1));
        double deg = std::abs(angle * 180.0 / CV_PI);
        if (deg < 15.0 || deg > 165.0) horizYs.push_back((y1 + y2) / 2);
        else if (std::abs(deg - 90.0) < 15.0) vertXs.push_back((x1 + x2) / 2);
    }

    int totalHough = static_cast<int>(horizYs.size()) + static_cast<int>(vertXs.size());
    if (totalHough < 4 && edgeDensity > 0.015) {
        outQuality = 0.0;
        return table;
    }

    int tolX = std::max(2, roiGray.cols / 40);
    int tolY = std::max(2, roiGray.rows / 40);
    auto xLines = clusterAndAverageCoordinates(vertXs, tolX);
    auto yLines = clusterAndAverageCoordinates(horizYs, tolY);

    if (xLines.size() < 2 || yLines.size() < 2) {
        outQuality = 0.0;
        if (debugger && debugger->enabled()) {
            try {
                std::ostringstream base; base << "opencv/" << imageStem << "/roi_" << roiOffset.x << "_" << roiOffset.y;
                std::ostringstream info;
                info << "overlapping_boxes=" << overlappingBoxes.size() << "\n";
                info << "raw_hough_lines=" << lines.size() << "\n";
                info << "horizYs=" << horizYs.size() << "\n";
                info << "vertXs=" << vertXs.size() << "\n";
                info << "clustered_xLines=" << xLines.size() << "\n";
                info << "clustered_yLines=" << yLines.size() << "\n";
                info << "edgeDensity=" << edgeDensity << "\n";
                info << "roiSize=" << roiGray.cols << "x" << roiGray.rows << "\n";
                info << "outCells=" << 0 << "\n";
                debugger->writeText(base.str() + "_debug.txt", info.str());
                writeImageViaDebugger(debugger, base.str() + "_work", work);
                writeImageViaDebugger(debugger, base.str() + "_edges", edges);
            } catch (...) {}
        }
        return table;
    }

    for (size_t r = 0; r + 1 < yLines.size(); ++r) {
        for (size_t c = 0; c + 1 < xLines.size(); ++c) {
            int x0 = xLines[c], x1 = xLines[c + 1];
            int y0 = yLines[r], y1 = yLines[r + 1];
            int w = x1 - x0, h = y1 - y0;
            if (w <= 6 || h <= 6) continue;
            cv::Rect crel(x0, y0, w, h);
            crel &= cv::Rect(0, 0, roiGray.cols, roiGray.rows);
            if (crel.width <= 0 || crel.height <= 0) continue;
            api::opencv::Cell cell;
            cell.bbox.x = crel.x + roiOffset.x; cell.bbox.y = crel.y + roiOffset.y; cell.bbox.width = crel.width; cell.bbox.height = crel.height;
            cell.row = static_cast<int>(r);
            cell.col = static_cast<int>(c);
            table.cells.push_back(cell);
        }
    }

    outCells = static_cast<int>(table.cells.size());
    if (outCells == 0) { outQuality = 0.0; return table; }

    double avgW = 0.0, avgH = 0.0;
    for (const auto& c : table.cells) { avgW += c.bbox.width; avgH += c.bbox.height; }
    avgW /= table.cells.size(); avgH /= table.cells.size();
    double sizeScore = std::clamp((avgW * avgH) / double(roiGray.cols * roiGray.rows), 0.0, 1.0);
    double lineScore = std::clamp((double)(xLines.size() + yLines.size()) / 20.0, 0.0, 1.0);
    outQuality = 0.6 * lineScore + 0.4 * sizeScore;

    table.bbox.x = roiOffset.x; table.bbox.y = roiOffset.y; table.bbox.width = roiOffset.width; table.bbox.height = roiOffset.height;
    if (!table.cells.empty()) {
        int minx = INT_MAX, miny = INT_MAX, maxx = 0, maxy = 0;
        for (const auto &c : table.cells) {
            minx = std::min(minx, c.bbox.x);
            miny = std::min(miny, c.bbox.y);
            maxx = std::max(maxx, c.bbox.x + c.bbox.width);
            maxy = std::max(maxy, c.bbox.y + c.bbox.height);
        }
        if (minx < maxx && miny < maxy) { table.bbox.x = minx; table.bbox.y = miny; table.bbox.width = maxx - minx; table.bbox.height = maxy - miny; }
        table.cols = static_cast<int>(xLines.size() > 0 ? xLines.size() - 1 : 0);
        table.rows = static_cast<int>(yLines.size() > 0 ? yLines.size() - 1 : 0);
    }

    if (debugger && debugger->enabled()) {
        try {
            std::ostringstream base; base << "opencv/" << imageStem << "/roi_" << roiOffset.x << "_" << roiOffset.y;
            std::ostringstream info;
            info << "overlapping_boxes=" << overlappingBoxes.size() << "\n";
            info << "raw_hough_lines=" << lines.size() << "\n";
            info << "horizYs=" << horizYs.size() << "\n";
            info << "vertXs=" << vertXs.size() << "\n";
            info << "clustered_xLines=" << xLines.size() << "\n";
            info << "clustered_yLines=" << yLines.size() << "\n";
            info << "edgeDensity=" << edgeDensity << "\n";
            info << "roiSize=" << roiGray.cols << "x" << roiGray.rows << "\n";
            info << "outCells=" << outCells << "\n";
            info << "outQuality=" << outQuality << "\n";
            debugger->writeText(base.str() + "_debug.txt", info.str());
            writeImageViaDebugger(debugger, base.str() + "_work", work);
            writeImageViaDebugger(debugger, base.str() + "_edges", edges);
        } catch (...) {}
    }

    return table;
}

std::string computeStem(const std::string& imagePath) {
    try { return std::filesystem::path(imagePath).stem().string(); } catch (...) { return "page"; }
}

} // namespace

std::vector<api::opencv::Table> DetectEngine::DetectTables(const cv::Mat& img, const std::string& imagePath, std::shared_ptr<IDebugger> debugger) {
    std::vector<api::opencv::Table> result;
    if (img.empty()) { if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Empty image: " + imagePath); return result; }

    std::string stem = computeStem(imagePath);
    auto blocks = findTextBlocksUsingMorphology(img, stem, debugger);

    struct Candidate { api::opencv::Table t; int cells; double q; };
    std::vector<Candidate> candidates;
    std::vector<cv::Rect> allGeneratedTextBoxes;

    for (const auto& b : blocks) {
        if (b.width < img.cols * 0.12) continue;
        cv::Mat roi = img(b);
        auto localBoxes = findTextBlocksUsingMorphology(roi, stem, debugger);
        std::vector<cv::Rect> pageLocalBoxes;
        pageLocalBoxes.reserve(localBoxes.size());
        for (const auto &lb : localBoxes) {
            if (lb.width >= static_cast<int>(roi.cols * 0.9) && lb.height >= static_cast<int>(roi.rows * 0.9)) continue;
            int padx = std::max(0, lb.width / 30);
            int pady = std::max(0, lb.height / 30);
            cv::Rect lbShrunk(lb.x + padx, lb.y + pady, std::max(1, lb.width - 2 * padx), std::max(1, lb.height - 2 * pady));
            cv::Rect pb(lbShrunk.x + b.x, lbShrunk.y + b.y, lbShrunk.width, lbShrunk.height);
            pageLocalBoxes.push_back(pb);
            allGeneratedTextBoxes.push_back(pb);
        }

        int cells = 0; double q = 0.0;
        auto tab = detectTableGridHough(roi, cv::Rect(b.x, b.y, b.width, b.height), pageLocalBoxes, cells, q, stem, debugger);
        if (cells > 0 && q > 0.05) candidates.push_back({ tab, cells, q });
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { if (a.cells != b.cells) return a.cells > b.cells; return a.q > b.q; });
    if (!candidates.empty()) result.push_back(candidates[0].t);

    if (debugger && debugger->enabled()) {
        try {
            cv::Mat vis; cv::cvtColor(img, vis, cv::COLOR_GRAY2BGR);
            for (const auto &t : result) cv::rectangle(vis, cv::Rect(t.bbox.x, t.bbox.y, t.bbox.width, t.bbox.height), cv::Scalar(0,255,0), 2);
            std::ostringstream oss; oss << "opencv/" << stem << "/detected_tables";
            writeImageViaDebugger(debugger, oss.str(), vis);

            if (!allGeneratedTextBoxes.empty()) {
                cv::Mat visBoxes; cv::cvtColor(img, visBoxes, cv::COLOR_GRAY2BGR);
                std::vector<cv::Scalar> colors = { cv::Scalar(0,0,255), cv::Scalar(0,255,0), cv::Scalar(255,0,0), cv::Scalar(0,255,255), cv::Scalar(255,0,255), cv::Scalar(255,255,0) };
                for (size_t i = 0; i < allGeneratedTextBoxes.size(); ++i) {
                    const auto &pb = allGeneratedTextBoxes[i];
                    cv::rectangle(visBoxes, pb, colors[i % colors.size()], 2);
                }
                std::ostringstream oss2; oss2 << "opencv/" << stem << "/textboxes_overlay";
                writeImageViaDebugger(debugger, oss2.str(), visBoxes);
            }
        } catch (...) {}
    }

    return result;
}

} // namespace opencv
