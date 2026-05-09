/**
 * @file core/include/core/ports/pdf-rendering/IPdfRenderer.h
 * @brief PDF rendering port for import workflows.
 */
#pragma once

#include "core/ports/pdf-rendering/PopplerRequest.h"
#include "core/ports/pdf-rendering/PopplerResult.h"

namespace core::ports::pdf_rendering {

/**
 * @brief Renders and extracts PDF content.
 */
class IPdfRenderer {
public:
    virtual ~IPdfRenderer() = default;

    /**
     * @brief Renders a PDF into images.
     * @param req Rendering request.
     * @return The rendering result.
     */
    virtual poppler::RenderResult render(const poppler::RenderRequest& req) = 0;

    /**
     * @brief Extracts page content from a PDF.
     * @param req Extraction request.
     * @return The extraction result.
     */
    virtual poppler::ExtractResult extract(const poppler::ExtractRequest& req) = 0;
};

} // namespace core::ports::pdf_rendering
