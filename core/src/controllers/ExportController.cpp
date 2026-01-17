#include "core/controllers/ExportController.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/XlsxController.h"

namespace core::controllers::exporting {

ExportController::ExportController(std::shared_ptr<XlsxController> xlsx, std::shared_ptr<CsvController> csv)
    : xlsx_(std::move(xlsx)), csv_(std::move(csv)) {}

bool ExportController::exportData(const ExportOptions& opts) {
    if (opts.format == ExportOptions::Format::Csv) {
        if (!csv_) return false;
        return csv_->exportData(opts);
    }
    if (opts.format == ExportOptions::Format::Xlsx) {
        if (!xlsx_) return false;
        return xlsx_->exportData(opts);
    }
    return false;
}

} // namespace core::controllers::exporting
