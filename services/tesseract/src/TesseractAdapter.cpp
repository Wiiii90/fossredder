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