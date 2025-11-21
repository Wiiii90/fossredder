#include "poppler/pch.h"
#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include "poppler/PopplerEngine.h"
#include "debug/IDebugger.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <sstream>

class PopplerAdapterImpl : public api::poppler::IPopplerAdapter {
  public:
    PopplerAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    api::poppler::RenderResult render(const api::poppler::RenderRequest& req) override {
        api::poppler::RenderResult out;
        try {
            if (debugger && debugger->enabled()) {
                try { debugger->writeText("poppler/log", std::string("render:start ") + req.pdfPath.string()); } catch (...) {}
            }

            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerEngine::renderDocument(req.pdfPath.string(), req.dpi, req.outputDir, debugger);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.images.reserve(pages.size());
            for (size_t i = 0; i < pages.size(); ++i) {
                out.images.push_back(std::filesystem::path(pages[i].imagePath));
            }

            if (debugger && debugger->enabled()) {
                for (size_t i = 0; i < pages.size(); ++i) {
                    try {
                        const std::string& filename = pages[i].imagePath;
                        std::ifstream ifs(filename, std::ios::binary);
                        if (ifs) {
                            std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                            try { debugger->writeBytes(std::string("poppler/output/") + std::filesystem::path(filename).stem().string() + std::string(".png"), buf); } catch (...) {}
                        }
                        try {
                            std::ostringstream oss;
                            oss << "render:page " << (i + 1) << " saved " << pages[i].imagePath;
                            debugger->writeText("poppler/log", oss.str());
                        } catch (...) {}
                    } catch (...) {}
                }
                try {
                    std::ostringstream oss;
                    oss << "render:done " << req.pdfPath.string() << " pages=" << pages.size() << " duration=" << dur.count();
                    debugger->writeText("poppler/log", oss.str());
                } catch (...) {}
            }
        } catch (const std::exception& ex) {
            if (debugger && debugger->enabled()) {
                try { debugger->writeText("poppler/log", std::string("render:error ") + ex.what()); } catch (...) {}
            }
            throw;
        }
        return out;
    }

    api::poppler::ExtractResult extract(const api::poppler::ExtractRequest& req) override {
        api::poppler::ExtractResult out;
        try {
            if (debugger && debugger->enabled()) {
                try { debugger->writeText("poppler/log", std::string("extract:start ") + req.pdfPath.string()); } catch (...) {}
            }
            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerEngine::extractDocumentMeta(req.pdfPath.string(), req.dpi, req.outputDir, debugger);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            nlohmann::json doc;
            nlohmann::json arr = nlohmann::json::array();
            for (size_t i = 0; i < pages.size(); ++i) {
                try {
                    if (!pages[i].metadataJson.empty()) {
                        try { debugger->writeText(std::string("poppler/meta/") + std::to_string(i + 1) + std::string(".json"), pages[i].metadataJson); } catch (...) {}
                        arr.push_back(nlohmann::json::parse(pages[i].metadataJson));
                    } else {
                        arr.push_back(nlohmann::json::object());
                    }
                } catch (...) {
                    arr.push_back(nlohmann::json::object());
                }
            }
            doc["pages"] = arr;
            out.metadata = doc;

            if (debugger && debugger->enabled()) {
                try {
                    std::ostringstream oss;
                    oss << "extract:done " << req.pdfPath.string() << " pages=" << pages.size() << " duration=" << dur.count();
                    debugger->writeText("poppler/log", oss.str());
                } catch (...) {}
            }
        } catch (const std::exception& ex) {
            if (debugger && debugger->enabled()) {
                try { debugger->writeText("poppler/log", std::string("extract:error ") + ex.what()); } catch (...) {}
            }
            throw;
        }
        return out;
    }

  private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<api::poppler::IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<PopplerAdapterImpl>(std::move(debugger));
}
