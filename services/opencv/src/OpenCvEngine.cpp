#include "opencv/pch.h"
#include "opencv/OpenCvEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::vector<cv::Rect> readTextBoxesFromJson(const std::string& imagePath) {
    std::vector<cv::Rect> boxes;
    try {
        std::filesystem::path p(imagePath);
        auto stem = p.stem().string();
        auto dir = p.parent_path();
        std::ostringstream js; js << (dir.empty() ? std::string() : dir.string() + std::string("/")) << stem << ".json";
        std::string jsonPath = js.str();
        if (!std::filesystem::exists(jsonPath)) return boxes;
        std::ifstream ifs(jsonPath);
        if (!ifs) return boxes;
        json j = json::parse(ifs);
        if (!j.contains("text_elements")) return boxes;
        for (const auto& te : j["text_elements"]) {
            try {
                if (te.contains("bbox_px")) {
                    auto arr = te["bbox_px"];
                    if (arr.is_array() && arr.size() >= 4) {
                        int x = arr[0].get<int>();
                        int y = arr[1].get<int>();
                        int w = arr[2].get<int>();
                        int h = arr[3].get<int>();
                        if (w > 0 && h > 0) boxes.emplace_back(x, y, w, h);
                    }
                }
            }
            catch (...) { }
        }
    }
    catch (...) { }
    return boxes;
}

static std::vector<cv::Rect> findTextBlocksUsingMorphology(const cv::Mat& gray) {
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
        if (rect.width > 50 && rect.height > 10) blocks.push_back(rect);
    }
    return blocks;
}

static std::vector<int> clusterAndAverageCoordinates(const std::vector<int>& coords, int tolerance) {
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

static void writeImageViaDebugger(std::shared_ptr<IDebugger> debugger, const std::string& relPath, const cv::Mat& img) {
    if (!debugger) return;
    std::vector<uint8_t> buf;
    try {
        bool ok = cv::imencode(".png", img, buf);
        if (!ok || buf.empty()) return;
        debugger->writeBytes(relPath + ".png", buf);
    } catch (...) {}
}

// include image stem so debug files are written under per-image subfolder
static api::opencv::Table detectTableGridHough(const cv::Mat& roiGray, const cv::Rect& roiOffset, const std::vector<cv::Rect>& pageTextBoxes, int& outCells, double& outQuality, const std::string& imageStem, std::shared_ptr<IDebugger> debugger) {
    api::opencv::Table table;
    outCells = 0; outQuality = 0.0;
    if (roiGray.empty()) return table;

    cv::Mat work = roiGray.clone();
    for (const auto& tb : pageTextBoxes) {
        cv::Rect rel = tb & cv::Rect(roiOffset.x, roiOffset.y, roiOffset.width, roiOffset.height);
        if (rel.width > 0 && rel.height > 0) {
            cv::Rect r(rel.x - roiOffset.x, rel.y - roiOffset.y, rel.width, rel.height);
            cv::rectangle(work, r, cv::Scalar(255), cv::FILLED);
        }
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
    if (totalHough < 6 && edgeDensity > 0.02) {
        outQuality = 0.0;
        return table;
    }

    int tolX = std::max(2, roiGray.cols / 40);
    int tolY = std::max(2, roiGray.rows / 40);
    auto xLines = clusterAndAverageCoordinates(vertXs, tolX);
    auto yLines = clusterAndAverageCoordinates(horizYs, tolY);

    if (xLines.size() < 2 || yLines.size() < 2) {
        outQuality = 0.0;
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
    }

    // debug outputs for this ROI
    if (debugger && debugger->enabled()) {
        try {
            std::ostringstream base; base << "opencv/" << imageStem << "/roi_" << roiOffset.x << "_" << roiOffset.y;
            std::ostringstream oss; oss << base.str() << "_gray";
            // recreate cv::Mat for output if needed
            writeImageViaDebugger(debugger, oss.str(), roiGray);
            // Also dump edges/hough overlay
            cv::Mat edges; cv::Canny(roiGray, edges, 50, 150);
            std::ostringstream oss2; oss2 << base.str() << "_edges";
            writeImageViaDebugger(debugger, oss2.str(), edges);
        } catch (...) {}
    }

    return table;
}

std::vector<api::opencv::Table> OpenCvEngine::detectTablesFromImage(const cv::Mat& img, const std::string& imagePath, int maxTables, std::shared_ptr<IDebugger> debugger) {
    std::vector<api::opencv::Table> result;
    if (img.empty()) { if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Empty image: " + imagePath); return result; }

    auto pageTextBoxes = readTextBoxesFromJson(imagePath);
    auto blocks = findTextBlocksUsingMorphology(img);

    struct Candidate { api::opencv::Table t; int cells; double q; };
    std::vector<Candidate> candidates;
    try { std::filesystem::path(imagePath).stem().string(); } catch (...) {}

    std::string stem;
    try { stem = std::filesystem::path(imagePath).stem().string(); } catch (...) { stem = "page"; }

    for (const auto& b : blocks) {
        if (b.width < img.cols * 0.12) continue;
        cv::Mat roi = img(b);
        int cells = 0; double q = 0.0;
        auto tab = detectTableGridHough(roi, cv::Rect(b.x, b.y, b.width, b.height), pageTextBoxes, cells, q, stem, debugger);
        if (cells > 0 && q > 0.05) candidates.push_back({ tab, cells, q });
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) { if (a.cells != b.cells) return a.cells > b.cells; return a.q > b.q; });
    int take = std::clamp(maxTables, 0, static_cast<int>(candidates.size()));
    for (int k = 0; k < take; ++k) result.push_back(candidates[k].t);

    if (debugger && debugger->enabled()) {
        // write overall debug overlay
        try {
            cv::Mat vis; cv::cvtColor(img, vis, cv::COLOR_GRAY2BGR);
            for (const auto &t : result) cv::rectangle(vis, cv::Rect(t.bbox.x, t.bbox.y, t.bbox.width, t.bbox.height), cv::Scalar(0,255,0), 2);
            std::ostringstream oss; oss << "opencv/" << stem << "/detected_tables";
            writeImageViaDebugger(debugger, oss.str(), vis);
        } catch (...) {}
    }

    return result;
}

static std::filesystem::path resolveCropOutputDir(const std::filesystem::path& outputDir, const std::string& stem) {
    if (!outputDir.empty()) return outputDir;
    try {
        auto tmp = std::filesystem::temp_directory_path() / "fossredder" / "opencv" / "output" / stem;
        std::filesystem::create_directories(tmp);
        return tmp;
    } catch (...) {
        return std::filesystem::current_path();
    }
}

std::vector<std::filesystem::path> OpenCvEngine::cropImages(const std::string& imagePath, const std::vector<api::opencv::Rect>& rects, const std::filesystem::path& outputDir, std::shared_ptr<IDebugger> debugger) {
    std::vector<std::filesystem::path> outPaths;
    if (rects.empty()) return outPaths;
    try {
        cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (img.empty()) {
            if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to load image for cropping: " + imagePath);
            return outPaths;
        }

        std::string stem;
        try { stem = std::filesystem::path(imagePath).stem().string(); } catch (...) { stem = "page"; }
        auto outDir = resolveCropOutputDir(outputDir, stem);
        std::filesystem::create_directories(outDir);

        int idx = 0;
        for (const auto &rct : rects) {
            try {
                cv::Rect r(rct.x, rct.y, rct.width, rct.height);
                r &= cv::Rect(0, 0, img.cols, img.rows);
                if (r.width <= 0 || r.height <= 0) { ++idx; continue; }
                cv::Mat crop = img(r).clone();
                std::ostringstream fname; fname << (outDir.string()) << "/" << (idx + 1) << ".png";
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
                                // write debugger copy under opencv/output/<stem>/<page>.png
                                std::ostringstream dbgRel;
                                dbgRel << "opencv/" << stem << "/" << (idx + 1 + "_cropped");
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
