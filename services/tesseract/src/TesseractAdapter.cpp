#include "tesseract/pch.h"
#include "api/tesseract/ITesseractAdapter.h"
#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResponse.h"
#include "api/tesseract/Types.h"
#include "tesseract/TesseractEngine.h"
#include "debug/IDebugger.h"
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cctype>

class TesseractAdapterImpl : public api::tesseract::ITesseractAdapter {
public:
    explicit TesseractAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    api::tesseract::ExtractResult extract(const api::tesseract::ExtractRequest& req) override {
        api::tesseract::ExtractResult out;
        std::vector<uint8_t> bytes;
        if (!req.imagePath.empty()) {
            try {
                std::ifstream ifs(req.imagePath, std::ios::binary);
                if (ifs) bytes.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            } catch (...) {}
        }

        auto [textDto, words] = TesseractEngine::extractFromBytes(bytes, req.tessdataPath, debugger);
        out.text = textDto.text;
        try {
            std::ostringstream oss;
            for (const auto &w : words) {
                oss << w.text << "\t" << w.bbox.x << "\t" << w.bbox.y << "\t" << w.bbox.width << "\t" << w.bbox.height << "\t" << w.confidence << "\n";
            }
            out.tsv = oss.str();
        } catch (...) { out.tsv.clear(); }

        out.words.reserve(words.size());
        for (const auto &w : words) {
            api::tesseract::Word wr;
            wr.bbox.x = w.bbox.x;
            wr.bbox.y = w.bbox.y;
            wr.bbox.width = w.bbox.width;
            wr.bbox.height = w.bbox.height;
            wr.text = w.text;
            wr.confidence = w.confidence;
            out.words.push_back(wr);
        }

        if (req.kind == api::tesseract::ExtractRequest::Kind::Table && !req.cells.empty()) {
            api::tesseract::Table t;
            t.cells = req.cells;

            auto inCell = [](const api::tesseract::Rect& w, const api::tesseract::Rect& c) {
                int cx = w.x + w.width / 2;
                int cy = w.y + w.height / 2;
                return cx >= c.x && cx < (c.x + c.width) && cy >= c.y && cy < (c.y + c.height);
            };

            struct WRef {
                const api::tesseract::Word* w;
                int cy;
            };

            for (auto& cell : t.cells) {
                std::vector<WRef> cellWords;
                cellWords.reserve(out.words.size() / 4);
                for (const auto& w : out.words) {
                    if (!inCell(w.bbox, cell.bbox)) continue;
                    cellWords.push_back(WRef{ &w, w.bbox.y + w.bbox.height / 2 });
                }

                if (cellWords.empty()) {
                    cell.text.clear();
                    cell.confidence = 0;
                    continue;
                }

                std::sort(cellWords.begin(), cellWords.end(), [](const WRef& a, const WRef& b) {
                    if (a.cy != b.cy) return a.cy < b.cy;
                    return a.w->bbox.x < b.w->bbox.x;
                });

                int avgH = 0;
                for (const auto& cw : cellWords) avgH += cw.w->bbox.height;
                avgH = std::max(1, avgH / static_cast<int>(cellWords.size()));
                int lineTol = std::max(4, avgH / 2);

                std::vector<std::vector<const api::tesseract::Word*>> lines;
                std::vector<int> lineY;

                for (const auto& cw : cellWords) {
                    if (lines.empty() || std::abs(cw.cy - lineY.back()) > lineTol) {
                        lines.push_back({ cw.w });
                        lineY.push_back(cw.cy);
                    } else {
                        lines.back().push_back(cw.w);
                        lineY.back() = (lineY.back() + cw.cy) / 2;
                    }
                }

                std::ostringstream text;
                int sumConf = 0;
                int n = 0;

                for (size_t li = 0; li < lines.size(); ++li) {
                    auto& ln = lines[li];
                    std::sort(ln.begin(), ln.end(), [](const auto* a, const auto* b) { return a->bbox.x < b->bbox.x; });
                    for (size_t wi = 0; wi < ln.size(); ++wi) {
                        if (wi) text << ' ';
                        text << ln[wi]->text;
                        sumConf += ln[wi]->confidence;
                        ++n;
                    }
                    if (li + 1 < lines.size()) text << "\n";
                }

                cell.text = text.str();
                cell.confidence = n > 0 ? (sumConf / n) : 0;
            }

            out.tables.push_back(std::move(t));
        }

        return out;
    }

private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter() {
    return std::make_shared<TesseractAdapterImpl>();
}

std::shared_ptr<api::tesseract::ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<TesseractAdapterImpl>(std::move(debugger));
}