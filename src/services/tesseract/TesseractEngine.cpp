#include "pch.h"
#include "services/tesseract/TesseractEngine.h"
#include "debug/IDebugger.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <opencv2/imgcodecs.hpp>

static std::string trimStrLocal(const std::string& s) {
    auto l = s.find_first_not_of(" \t\n\r");
    if (l == std::string::npos) return std::string();
    auto r = s.find_last_not_of(" \t\n\r");
    return s.substr(l, r - l + 1);
}

static void initTessLocal(tesseract::TessBaseAPI& ocr, const std::string& tessdataPath) {
    ocr.SetVariable("TESSDATA_PREFIX", tessdataPath.c_str());
    if (ocr.Init(tessdataPath.c_str(), "deu") != 0) {
        throw std::runtime_error("Failed to initialize Tesseract OCR with German language.");
    }
}

static OcrTextDto fromTessTextLocal(tesseract::TessBaseAPI& ocr) {
    OcrTextDto out;
    char* txt = ocr.GetUTF8Text();
    if (txt) { out.text = std::string(txt); delete[] txt; }
    out.meanConfidence = ocr.MeanTextConf();
    return out;
}

static OcrWordsDto parseTsvToWordsLocal(const std::string& tsv, int pad) {
    OcrWordsDto result;
    std::istringstream iss(tsv);
    std::string line;
    bool first = true;
    int sumConf = 0;
    int count = 0;
    while (std::getline(iss, line)) {
        if (first) { first = false; continue; }
        if (line.empty()) continue;
        std::vector<std::string> cols;
        std::istringstream ls(line);
        std::string cell;
        while (std::getline(ls, cell, '\t')) cols.push_back(cell);
        if (cols.size() < 12) continue;
        int level = std::stoi(cols[0]);
        if (level != 5) continue;
        int left = std::stoi(cols[6]);
        int top = std::stoi(cols[7]);
        int width = std::stoi(cols[8]);
        int height = std::stoi(cols[9]);
        int conf = std::stoi(cols[10]);
        std::string text = cols[11];
        OcrWordDto w;
        w.bbox.x = left - pad; w.bbox.y = top - pad; w.bbox.width = width; w.bbox.height = height;
        w.text = text;
        w.confidence = conf;
        result.words.push_back(w);
        sumConf += conf; ++count;
    }
    result.meanConfidence = count == 0 ? 0 : sumConf / count;
    return result;
}

static Pix* pixFromBytesLocal(const OcrRequest& input) {
    if ((input.width <= 0 || input.height <= 0 || input.channels <= 0) && !input.data.empty()) {
        Pix* p = pixReadMem(input.data.data(), static_cast<size_t>(input.data.size()));
        return p;
    }

    if (input.data.empty() || input.width <= 0 || input.height <= 0 || input.channels <= 0) return nullptr;
    int w = input.width;
    int h = input.height;
    int channels = input.channels;
    int bytesPerRow = input.step > 0 ? input.step : (w * channels);

    if (channels == 1) {
        Pix* p = pixCreate(w, h, 8);
        if (!p) return nullptr;
        uint8_t* destBytes = reinterpret_cast<uint8_t*>(pixGetData(p));
        int wpl = pixGetWpl(p);
        for (int y = 0; y < h; ++y) {
            uint8_t* destLine = destBytes + y * wpl * 4;
            const uint8_t* srcLine = input.data.data() + y * bytesPerRow;
            memcpy(destLine, srcLine, static_cast<size_t>(w));
        }
        return p;
    }
    else if (channels == 3 || channels == 4) {
        Pix* p = pixCreate(w, h, 32);
        if (!p) return nullptr;
        l_uint32* dest = pixGetData(p);
        int wpl = pixGetWpl(p);
        for (int y = 0; y < h; ++y) {
            const uint8_t* src = input.data.data() + y * bytesPerRow;
            l_uint32* destRow = dest + y * wpl;
            for (int x = 0; x < w; ++x) {
                uint8_t b = src[x * channels + 0];
                uint8_t g = src[x * channels + 1];
                uint8_t r = src[x * channels + 2];
                l_uint32 pixel = (static_cast<l_uint32>(r) << 24) | (static_cast<l_uint32>(g) << 16) | (static_cast<l_uint32>(b) << 8);
                destRow[x] = pixel;
            }
            for (int x = w; x < wpl; ++x) destRow[x] = 0;
        }
        return p;
    }
    return nullptr;
}

// helper to write image bytes via debugger using cv::imencode
static void writeImageViaDebugger(std::shared_ptr<IDebugger> debugger, const std::string& relPath, const cv::Mat& img) {
    if (!debugger) return;
    std::vector<uint8_t> buf;
    try {
        cv::imencode(".png", img, buf);
        debugger->writeBytes(relPath + ".png", buf);
    } catch (...) {}
}

OcrTextDto TesseractEngine::recognizeText(const OcrRequest& input, std::shared_ptr<IDebugger> debugger) {
    OcrTextDto out;
    tesseract::TessBaseAPI ocr;
    initTessLocal(ocr, input.tessdataPath);
    Pix* image = pixFromBytesLocal(input);
    if (!image) return out;
    ocr.SetImage(image);
    int dpi = pixGetXRes(image);
    if (dpi <= 0) dpi = 300;
    ocr.SetSourceResolution(dpi);
    ocr.SetPageSegMode(static_cast<tesseract::PageSegMode>(input.psm));
    ocr.Recognize(0);
    out = fromTessTextLocal(ocr);

    // debug: write text output into structured tesseract/output folder
    if (debugger && debugger->enabled()) {
        std::string prefix = input.debugPrefix.empty() ? "page" : input.debugPrefix;
        debugger->writeText(std::string("tesseract/output/") + prefix + "_text.txt", out.text);
    }

    ocr.End();
    pixDestroy(&image);
    return out;
}

OcrWordsDto TesseractEngine::getWords(const OcrRequest& input, std::shared_ptr<IDebugger> debugger) {
    OcrWordsDto out;
    Pix* image = pixFromBytesLocal(input);
    if (!image) return out;
    tesseract::TessBaseAPI ocr;
    initTessLocal(ocr, input.tessdataPath);
    ocr.SetImage(image);
    int dpi = pixGetXRes(image);
    if (dpi <= 0) dpi = 300;
    ocr.SetSourceResolution(dpi);
    ocr.SetPageSegMode(static_cast<tesseract::PageSegMode>(input.psm));
    ocr.Recognize(0);
    char* tsv = ocr.GetTSVText(0);
    if (!tsv) { ocr.End(); pixDestroy(&image); return out; }
    std::string s(tsv);

    // debug: write TSV and masked image into structured tesseract folders
    if (debugger && debugger->enabled()) {
        try {
            std::string prefix = input.debugPrefix.empty() ? "page" : input.debugPrefix;
            debugger->writeText(std::string("tesseract/tsv/") + prefix + "_words.tsv", s);
            // write masked image: convert pix to cv::Mat and write
            try {
                l_int32 w = 0, h = 0;
                pixGetDimensions(image, &w, &h, nullptr);
                if (w > 0 && h > 0) {
                    l_uint32* data = pixGetData(image);
                    int wpl = pixGetWpl(image);
                    cv::Mat mat(h, w, CV_8UC4);
                    for (int y = 0; y < h; ++y) {
                        l_uint32* row = data + y * wpl;
                        uint8_t* dst = mat.ptr<uint8_t>(y);
                        for (int x = 0; x < w; ++x) {
                            l_uint32 px = row[x];
                            dst[x*4 + 0] = (px >> 8) & 0xFF; // b
                            dst[x*4 + 1] = (px >> 16) & 0xFF; // g
                            dst[x*4 + 2] = (px >> 24) & 0xFF; // r
                            dst[x*4 + 3] = 255;
                        }
                    }
                    writeImageViaDebugger(debugger, std::string("tesseract/output/") + prefix + "_masked", mat);
                }
            } catch (...) {}
        } catch (...) {}
    }

    out = parseTsvToWordsLocal(s, /*pad*/0);
    delete[] tsv;
    ocr.End();
    pixDestroy(&image);
    return out;
}

OcrTableDto TesseractEngine::extractTable(const OcrRequest& input, std::shared_ptr<IDebugger> debugger) {
    OcrTableDto out;
    OcrWordsDto words = getWords(input, debugger);

    if (!input.templateCells.empty()) {
        out.bbox = input.tableBbox;
        std::vector<double> centersY, centersX;
        centersY.reserve(input.templateCells.size()); centersX.reserve(input.templateCells.size());
        for (const auto& t : input.templateCells) {
            double cy = t.y + t.height / 2.0;
            double cx = t.x + t.width / 2.0;
            centersY.push_back(cy);
            centersX.push_back(cx);
        }

        double tolY = 10.0, tolX = 10.0;
        {
            std::vector<int> heights, widths;
            heights.reserve(input.templateCells.size()); widths.reserve(input.templateCells.size());
            for (const auto& t : input.templateCells) { heights.push_back(t.height); widths.push_back(t.width); }
            if (!heights.empty()) {
                sort(heights.begin(), heights.end()); sort(widths.begin(), widths.end());
                int midh = heights[heights.size()/2]; int midw = widths[widths.size()/2];
                if (midh > 0) tolY = std::max(5.0, midh / 3.0);
                if (midw > 0) tolX = std::max(5.0, midw / 3.0);
            }
        }

        auto clusterCenters = [&](std::vector<double> vals, double tol) {
            std::vector<double> outv;
            if (vals.empty()) return outv;
            sort(vals.begin(), vals.end());
            double sum = vals[0]; int cnt = 1;
            for (size_t i = 1; i < vals.size(); ++i) {
                double v = vals[i];
                if (v - vals[i-1] <= tol) { sum += v; ++cnt; }
                else { outv.push_back(sum / cnt); sum = v; cnt = 1; }
            }
            outv.push_back(sum / cnt); return outv;
        };

        auto rowCenters = clusterCenters(centersY, tolY);
        auto colCenters = clusterCenters(centersX, tolX);

        out.rows = static_cast<int>(rowCenters.size());
        out.cols = static_cast<int>(colCenters.size());

        if (!input.debugPrefix.empty() && debugger && debugger->enabled()) {
            std::ostringstream wofs_ss;
            for (size_t wi = 0; wi < words.words.size(); ++wi) {
                const auto &w = words.words[wi];
                std::ostringstream L; L << "w#" << wi << " bbox=(" << w.bbox.x << "," << w.bbox.y << "," << w.bbox.width << "," << w.bbox.height << ") conf=" << w.confidence << " text='" << w.text << "'";
                wofs_ss << L.str() << "\n";
            }
            std::string prefix = input.debugPrefix.empty() ? "page" : input.debugPrefix;
            debugger->writeText(std::string("tesseract/debug/") + prefix + "_words_debug.txt", wofs_ss.str());
        }

        // collect header duplication debug messages
        std::ostringstream headerDebug_ss;

        for (size_t i = 0; i < input.templateCells.size(); ++i) {
            const auto& t = input.templateCells[i];
            double cy = t.y + t.height / 2.0;
            int rowIdx = 0; double minDist = 1e9;
            for (size_t r = 0; r < rowCenters.size(); ++r) { double d = std::abs(rowCenters[r] - cy); if (d < minDist) { minDist = d; rowIdx = static_cast<int>(r); } }
            double cx = t.x + t.width / 2.0; int colIdx = 0; minDist = 1e9;
            for (size_t c = 0; c < colCenters.size(); ++c) { double d = std::abs(colCenters[c] - cx); if (d < minDist) { minDist = d; colIdx = static_cast<int>(c); } }
            OcrCellDto c;
            c.bbox = t;
            c.row = rowIdx; c.col = colIdx; c.text.clear(); c.confidence = 0;
            out.cells.push_back(c);
        }

        std::vector<std::vector<int>> assignedIdx(out.cells.size());
        for (size_t wi = 0; wi < words.words.size(); ++wi) {
            const auto &w = words.words[wi];
            for (size_t ci = 0; ci < out.cells.size(); ++ci) {
                auto &cell = out.cells[ci];
                if ((w.bbox.x + w.bbox.width/2) >= cell.bbox.x && (w.bbox.x + w.bbox.width/2) <= cell.bbox.x + cell.bbox.width && (w.bbox.y + w.bbox.height/2) >= cell.bbox.y && (w.bbox.y + w.bbox.height/2) <= cell.bbox.y + cell.bbox.height) {
                    assignedIdx[ci].push_back(static_cast<int>(wi));
                    if (!cell.text.empty()) cell.text += " ";
                    cell.text += w.text;
                    cell.confidence = (cell.confidence + w.confidence) / 2;
                }
            }
        }

        if (!out.cells.empty()) {
            double firstRowCenter = 1e9; int medianCellH = 0;
            std::vector<int> hs; for (const auto &c : out.cells) hs.push_back(c.bbox.height);
            if (!hs.empty()) { sort(hs.begin(), hs.end()); medianCellH = hs[hs.size()/2]; }
            double minCenter = 1e9; for (const auto &c : out.cells) { double cy = c.bbox.y + c.bbox.height/2.0; if (cy < minCenter) minCenter = cy; }
            firstRowCenter = minCenter; double headerTol = std::max(10.0, medianCellH / 2.0);
            int cols = out.cols > 0 ? out.cols : 1;
            struct ColB { int minx = INT_MAX; int maxx = 0; };
            std::vector<ColB> colBounds(cols);
            for (const auto &c : out.cells) {
                if (c.col >= 0 && c.col < cols) { colBounds[c.col].minx = std::min(colBounds[c.col].minx, c.bbox.x); colBounds[c.col].maxx = std::max(colBounds[c.col].maxx, c.bbox.x + c.bbox.width); }
            }

            double headerBand = std::min((double)input.tableBbox.height / 3.0, std::max((double)medianCellH * 3.0, (double)medianCellH));
            for (size_t wi = 0; wi < words.words.size(); ++wi) {
                const auto &w = words.words[wi];
                double wyc = w.bbox.y + w.bbox.height / 2.0;
                if (wyc >= firstRowCenter - headerTol) continue;
                if (wyc > headerBand) continue;
                for (int col = 0; col < cols; ++col) {
                    int cx0 = colBounds[col].minx; int cx1 = colBounds[col].maxx;
                    if (cx0 == INT_MAX) continue;
                    int wx0 = w.bbox.x; int wx1 = w.bbox.x + w.bbox.width;
                    int ov = std::min(cx1, wx1) - std::max(cx0, wx0);
                    if (ov > 0) {
                        for (auto &cell : out.cells) {
                            if (cell.col == col) {
                                std::string headerText = trimStrLocal(w.text);
                                if (!headerText.empty()) {
                                    if (!cell.text.empty()) cell.text = headerText + " " + cell.text;
                                    else cell.text = headerText;
                                    cell.confidence = std::max(cell.confidence, w.confidence);
                                }
                            }
                        }
                        if (!input.debugPrefix.empty() && debugger && debugger->enabled()) {
                            std::ostringstream msg; msg << "Header word '" << w.text << "' duplicated to column " << col << "\n";
                            headerDebug_ss << msg.str();
                        }
                    }
                }
            }

            if (!input.debugPrefix.empty() && debugger && debugger->enabled()) {
                // write cells debug via debugger
                try {
                    std::ostringstream ss;
                    for (size_t ci = 0; ci < out.cells.size(); ++ci) {
                        auto &cell = out.cells[ci];
                        cell.text = trimStrLocal(cell.text);
                        std::ostringstream line;
                        line << "row=" << cell.row << " col=" << cell.col << " bbox=(" << cell.bbox.x << "," << cell.bbox.y << "," << cell.bbox.width << "," << cell.bbox.height << ")";
                        line << " conf=" << cell.confidence << " text='" << cell.text << "'";
                        ss << line.str() << "\n";
                    }
                    std::string prefix = input.debugPrefix.empty() ? "page" : input.debugPrefix;
                    debugger->writeText(std::string("tesseract/debug/") + prefix + "_cells.txt", ss.str());
                    // write header duplication messages if any
                    std::string hdr = headerDebug_ss.str(); if (!hdr.empty()) debugger->writeText(std::string("tesseract/debug/") + prefix + "_header_dup.txt", hdr);
                } catch (...) {}
            }
        }

        out.words = words.words;
        return out;
    }

    // Fallback segmentation logic
    std::vector<const OcrWordDto*> all;
    for (const auto& w : words.words) all.push_back(&w);
    sort(all.begin(), all.end(), [](const OcrWordDto* a, const OcrWordDto* b) { return a->bbox.y < b->bbox.y; });

    std::vector<std::vector<const OcrWordDto*>> rows;
    const int rowTolerance = 10;
    for (const auto* w : all) {
        if (rows.empty()) rows.push_back({ w });
        else {
            const auto& lastRow = rows.back();
            int avgY = 0; for (const auto* rw : lastRow) avgY += rw->bbox.y; avgY /= static_cast<int>(lastRow.size());
            if (std::abs(w->bbox.y - avgY) <= rowTolerance) rows.back().push_back(w);
            else rows.push_back({ w });
        }
    }

    out.cells.clear(); out.rows = static_cast<int>(rows.size()); out.cols = 1;
    for (size_t r = 0; r < rows.size(); ++r) {
        int minx = INT_MAX, miny = INT_MAX, maxx = 0, maxy = 0, sumConf = 0; std::string text;
        for (const auto* w : rows[r]) {
            minx = std::min(minx, w->bbox.x); miny = std::min(miny, w->bbox.y); maxx = std::max(maxx, w->bbox.x + w->bbox.width); maxy = std::max(maxy, w->bbox.y + w->bbox.height);
            if (!text.empty()) text += " "; text += w->text; sumConf += w->confidence;
        }
        OcrCellDto c; c.bbox.x = minx; c.bbox.y = miny; c.bbox.width = maxx - minx; c.bbox.height = maxy - miny; c.row = static_cast<int>(r); c.col = 0; c.text = text; c.confidence = rows[r].empty() ? 0 : sumConf / static_cast<int>(rows[r].size());
        out.cells.push_back(c);
    }

    out.words = words.words;
    if (!input.debugPrefix.empty() && debugger && debugger->enabled()) {
        std::ostringstream ss;
        for (const auto& cell : out.cells) {
            std::ostringstream line; line << "row=" << cell.row << " col=" << cell.col << " bbox=(" << cell.bbox.x << "," << cell.bbox.y << "," << cell.bbox.width << "," << cell.bbox.height << ")";
            line << " conf=" << cell.confidence << " text='" << cell.text << "'";
            ss << line.str() << "\n";
        }
        std::string prefix = input.debugPrefix.empty() ? "page" : input.debugPrefix;
        debugger->writeText(std::string("tesseract/debug/") + prefix + "_cells.txt", ss.str());
    }

    return out;
}
