#include "pch.h"
#include "services/poppler/PopplerEngine.h"
#include "services/poppler/PopplerDTO.h"
#include "debug/IDebugger.h"
#include "utils/Env.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-page-renderer.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

static PopplerRenderedPage extractPageMeta(poppler::page* page, int pageIndex, double dpi, std::shared_ptr<IDebugger> debugger) {
    PopplerRenderedPage rp;
    double pageWidthPts = 0.0, pageHeightPts = 0.0;
    try {
        auto rect = page->page_rect();
        pageWidthPts = rect.width();
        pageHeightPts = rect.height();
    } catch (...) {
        pageWidthPts = 595.2; pageHeightPts = 841.8;
    }

    rp.pageWidthPts = pageWidthPts;
    rp.pageHeightPts = pageHeightPts;
    rp.dpiX = dpi; rp.dpiY = dpi;

    try {
        auto tbl = page->text_list();
        for (const auto& tb : tbl) {
            PopplerTextElement te;
            try {
                auto u = tb.text();
                auto ba = u.to_utf8();
                if (ba.size() > 0 && ba.data() != nullptr)
                    te.text.assign(reinterpret_cast<const char*>(ba.data()), ba.size());
            } catch (...) { te.text.clear(); }

            try {
                auto b = tb.bbox();
                try { te.x = b.x(); te.y = b.y(); te.width = b.width(); te.height = b.height(); }
                catch (...) {
                    try { te.x = b.left(); te.y = b.top(); te.width = b.right() - b.left(); te.height = b.bottom() - b.top(); }
                    catch (...) { te.x = te.y = te.width = te.height = 0.0; }
                }
            } catch (...) { te.x = te.y = te.width = te.height = 0.0; }

            rp.textElements.push_back(te);
        }
    } catch (...) {}

    json j;
    j["dpi_x"] = dpi; j["dpi_y"] = dpi;
    j["page_width_pts"] = pageWidthPts; j["page_height_pts"] = pageHeightPts;
    double scaleX = dpi / 72.0; double scaleY = dpi / 72.0;
    json tj = json::array();
    for (const auto& te : rp.textElements) {
        json it;
        it["text"] = te.text;
        it["bbox_pts"] = { te.x, te.y, te.width, te.height };
        it["bbox_px"] = { static_cast<int>(std::round(te.x * scaleX)), static_cast<int>(std::round(te.y * scaleY)), static_cast<int>(std::round(te.width * scaleX)), static_cast<int>(std::round(te.height * scaleY)) };
        tj.push_back(it);
    }
    j["text_elements"] = tj;
    j["embedded_images"] = json::array();
    rp.metadataJson = j.dump();

    if (debugger && debugger->enabled()) {
        try {
            std::string stem = std::to_string(pageIndex + 1);
            debugger->writeText(std::string("poppler/meta/") + stem + ".json", rp.metadataJson);
        } catch (...) {}
    }

    return rp;
}

std::vector<PopplerRenderedPage> PopplerEngine::extractDocumentMeta(const std::string& pdfPath, double dpi, std::shared_ptr<IDebugger> debugger) {
    std::vector<PopplerRenderedPage> result;
    if (!std::filesystem::exists(pdfPath)) throw std::runtime_error("PDF file does not exist: " + pdfPath);

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdfPath));
    if (!doc) throw std::runtime_error("Failed to open PDF: " + pdfPath);

    int numPages = doc->pages();
    for (int i = 0; i < numPages; ++i) {
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (!page) continue;
        result.push_back(extractPageMeta(page.get(), i, dpi, debugger));
    }
    return result;
}

std::vector<PopplerRenderedPage> PopplerEngine::renderDocument(const std::string& pdfPath, double dpi, std::shared_ptr<IDebugger> debugger) {
    std::vector<PopplerRenderedPage> result;
    if (!std::filesystem::exists(pdfPath)) throw std::runtime_error("PDF file does not exist: " + pdfPath);

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdfPath));
    if (!doc) throw std::runtime_error("Failed to open PDF: " + pdfPath);

    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    renderer.set_image_format(poppler::image::format_enum::format_rgb24);

    int numPages = doc->pages();

    // sensible progress: report total pages once
    if (debugger && debugger->enabled()) {
        try {
            std::ostringstream oss;
            oss << "render:pages " << pdfPath << " total=" << numPages;
            debugger->writeText("poppler/log", oss.str());
        } catch (...) {}
    }

    for (int i = 0; i < numPages; ++i) {
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (!page) continue;

        poppler::image img = renderer.render_page(page.get(), dpi, dpi);
        if (!img.is_valid()) continue;

        std::filesystem::path outDir = env::getUserCacheDir("FOSSredder") / "poppler" / "output";
        std::filesystem::create_directories(outDir);
        std::ostringstream fname; fname << (outDir.string()) << "/" << (i + 1) << ".png";
        std::string filename = fname.str();

        if (!img.save(filename, "png")) throw std::runtime_error("Failed to save image: " + filename);

        PopplerRenderedPage rp; rp.imagePath = filename; rp.dpiX = dpi; rp.dpiY = dpi;

        if (debugger && debugger->enabled()) {
            try {
                std::ifstream ifs(filename, std::ios::binary);
                if (ifs) {
                    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                    debugger->writeBytes(std::string("poppler/output/") + std::filesystem::path(filename).stem().string() + std::string(".png"), buf);
                }
                // sensible per-page progress log after successfully writing bytes
                try {
                    std::ostringstream oss;
                    oss << "render:page " << (i + 1) << " saved " << filename;
                    debugger->writeText("poppler/log", oss.str());
                } catch (...) {}
            } catch (...) {}
        }

        result.push_back(std::move(rp));
    }
    return result;
}
