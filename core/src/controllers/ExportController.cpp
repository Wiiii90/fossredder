/**
 * @file core/src/controllers/ExportController.cpp
 * @brief Dispatches export requests to the format-specific CsvController / XlsxController.
 */
#include "core/controllers/ExportController.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/XlsxController.h"
#include "core/constants/CoreDefaults.h"

namespace core::controllers::exporting {

ExportResult ExportController::exportData(const ExportRequest& request) const
{
    switch (request.format) {
    case ExportFormat::Csv:
        return CsvController{}.exportData(request);
    case ExportFormat::Xlsx:
        return XlsxController{}.exportData(request);
    }
    return ExportResult{
        ExportStatus::UnsupportedFormat,
        request.format,
        {},
        std::string(core::constants::exportFlow::kErrorUnsupportedFormat),
        std::string(core::constants::exportFlow::kMessageUnsupportedFormat)
    };
}

} // namespace core::controllers::exporting
