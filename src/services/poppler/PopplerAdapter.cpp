#include "pch.h"
#include "services/poppler/IPopplerAdapter.h"
#include "services/poppler/PopplerEngine.h"
#include "debug/IDebugger.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>

class PopplerAdapterImpl : public IPopplerAdapter {
public:
    PopplerAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    RenderResult renderDocument(const RenderRequest& req) override {
        RenderResult out;
        try {
            if (debugger && debugger->enabled()) {
                try {
                    debugger->writeText("poppler/log", std::string("render:start ") + req.pdfPath.string());
                } catch (...) {}
            }

            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerEngine::renderDocument(req.pdfPath.string(), req.dpi, debugger);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.images.reserve(pages.size());
            for (size_t i = 0; i < pages.size(); ++i) {
                out.images.push_back(std::filesystem::path(pages[i].imagePath));
            }

            if (debugger && debugger->enabled()) {
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

    ExtractResult extractDocumentMeta(const ExtractRequest& req) override {
        ExtractResult out;
        try {
            if (debugger && debugger->enabled()) {
                try { debugger->writeText("poppler/log", std::string("extract:start ") + req.pdfPath.string()); } catch (...) {}
            }
            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerEngine::extractDocumentMeta(req.pdfPath.string(), req.dpi, debugger);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            try {
                nlohmann::json doc;
                nlohmann::json arr = nlohmann::json::array();
                for (size_t i = 0; i < pages.size(); ++i) {
                    try {
                        if (!pages[i].metadataJson.empty()) {
                            nlohmann::json pj = nlohmann::json::parse(pages[i].metadataJson);
                            arr.push_back(pj);
                        } else {
                            arr.push_back(nlohmann::json::object());
                        }
                    } catch (...) {
                        arr.push_back(nlohmann::json::object());
                    }
                }
                doc["pages"] = arr;
                out.metadata = doc;
            } catch (...) {
                out.metadata = nlohmann::json::object();
            }

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

std::shared_ptr<IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<PopplerAdapterImpl>(std::move(debugger));
}
