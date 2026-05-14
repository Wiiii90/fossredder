/**
 * @file core/include/core/application/export/ExportService.h
 * @brief Declares the export entry point used by the application shell.
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
 * @brief Dispatches export requests to the format-specific exporters.
 */
class ExportService {
public:
    explicit ExportService(std::shared_ptr<core::ports::archive::IArchive> archive = {},
                           std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter = {},
                           std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer = {});

    /**
     * @brief Dispatches an export request to the appropriate exporter implementation.
     * @param request Export request describing the desired output.
     * @return Export result describing success or failure.
     */
    ExportResult exportData(const ExportRequest& request) const;

private:
    std::shared_ptr<core::ports::archive::IArchive> archive_;
    std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter_;
    std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer_;
};

} // namespace core::application::exporting
