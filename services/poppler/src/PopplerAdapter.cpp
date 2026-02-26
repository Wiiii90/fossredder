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
            auto pages = PopplerEngine::renderDocument(req.pdfPath.string(), req.dpi, req.outputDir, req.uniqIdPrefix, req.filePrefix, debugger, req.cancelFlag);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.images.clear();
            out.imageBytes.clear();
            out.images.reserve(pages.size());
            out.imageBytes.reserve(pages.size());
            for (const auto& p : pages) {
                out.images.push_back(p.imagePath.empty() ? std::filesystem::path() : std::filesystem::path(p.imagePath));
                out.imageBytes.push_back(p.imageBytes);
            }

            if (debugger && debugger->enabled()) {
                try {
                    std::ostringstream oss;
                    oss << "render:done " << req.pdfPath.string() << " pages=" << out.images.size() << " duration=" << dur.count();
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
            auto pages = PopplerEngine::extractDocumentMeta(req.pdfPath.string(), req.dpi, req.outputDir, req.uniqIdPrefix, req.filePrefix, debugger, req.cancelFlag);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.pages = std::move(pages);

            if (debugger && debugger->enabled()) {
                try {
                    std::ostringstream oss;
                    oss << "extract:done " << req.pdfPath.string() << " pages=" << out.pages.size() << " duration=" << dur.count();
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
