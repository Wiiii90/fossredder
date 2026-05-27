#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

#include <memory>

namespace core::ports::archive { class IArchive; }
namespace core::ports::xlsx_writer { class IXlsxWriter; }
namespace core::ports::analysis_image_renderer { class IAnalysisImageRenderer; }

namespace core::application::exporting {

ExportResult exportObjectRequests(
    const ExportRequest& request,
    const std::shared_ptr<core::ports::archive::IArchive>& archive,
    const std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter>& xlsxWriter,
    const std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer>& imageRenderer);

} // namespace core::application::exporting

