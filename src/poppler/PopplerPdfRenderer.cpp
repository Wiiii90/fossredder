#include "pch.h"
#include "poppler/PopplerPdfRenderer.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-page-renderer.h>

std::vector<std::string> PopplerPdfRenderer::renderToImages(const std::string& pdfPath, const std::string& outputPrefix) {
    if (!std::filesystem::exists(pdfPath)) {
        throw std::runtime_error("PDF file does not exist: " + pdfPath);
    }

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdfPath));
    if (!doc) {
        throw std::runtime_error("Failed to open PDF: " + pdfPath);
    }

    int numPages = doc->pages();
    std::vector<std::string> imageFiles;
    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    renderer.set_image_format(poppler::image::format_enum::format_rgb24);

    for (int i = 0; i < numPages; ++i) {
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (!page) continue;

        poppler::image img = renderer.render_page(page.get(), 300.0, 300.0);
        if (!img.is_valid()) continue;

        std::string filename = outputPrefix + std::to_string(i + 1) + ".png";
        if (!img.save(filename, "png")) {
            throw std::runtime_error("Failed to save image: " + filename);
        }
        imageFiles.push_back(filename);
    }
    return imageFiles;
}