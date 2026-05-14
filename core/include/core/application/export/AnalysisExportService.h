/**
 * @file core/include/core/application/export/AnalysisExportService.h
 * @brief Declares the export service for analysis outputs and optional packaging.
 */

#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

namespace core::ports::archive {
class IArchive;
}
namespace core::ports::analysis_image_renderer {
class IAnalysisImageRenderer;
}
namespace core::ports::xlsx_writer {
class IXlsxWriter;
}

namespace core::application::exporting {

/**
 * @brief Writes configured analysis outputs (CSV/XLSX/JPG/PNG) and applies package mode.
 */
class AnalysisExportService {
public:
    explicit AnalysisExportService(std::shared_ptr<core::ports::archive::IArchive> archive = {},
                                   std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter = {},
                                   std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer = {});

    /**
     * @brief Exports the configured analysis items and optionally packages them into an archive.
     * @param request Export request describing output location, formats, and source snapshot.
     * @return Export result describing success or failure.
     */
    ExportResult exportAnalyses(const ExportRequest& request) const;

private:
    std::shared_ptr<core::ports::archive::IArchive> archive_;
    std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter_;
    std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer_;
};

} // namespace core::application::exporting
