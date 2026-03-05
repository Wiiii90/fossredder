#include "core/controllers/ExportController.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/XlsxController.h"

namespace core::controllers::exporting {

ExportController::ExportController(std::shared_ptr<XlsxController> xlsx, std::shared_ptr<CsvController> csv)
    : xlsx_(std::move(xlsx)), csv_(std::move(csv)) {}

ExportOptions::Status ExportController::exportData(ExportOptions& opts) {
    opts.actualFormat = opts.requestedFormat;
    opts.resolvedOutputPath = opts.outputPath;
    opts.errorCode.clear();
    opts.message.clear();

    if (opts.requestedFormat == ExportOptions::Format::Csv) {
        if (!csv_) {
            opts.status = ExportOptions::Status::InternalError;
            opts.errorCode = "EXPORT_CSV_CONTROLLER_MISSING";
            opts.message = "CSV controller is not available";
            return opts.status;
        }
        return csv_->exportData(opts);
    }
    if (opts.requestedFormat == ExportOptions::Format::Xlsx) {
        if (!xlsx_) {
            opts.status = ExportOptions::Status::InternalError;
            opts.errorCode = "EXPORT_XLSX_CONTROLLER_MISSING";
            opts.message = "XLSX controller is not available";
            return opts.status;
        }
        return xlsx_->exportData(opts);
    }

    opts.status = ExportOptions::Status::UnsupportedFormat;
    opts.errorCode = "EXPORT_UNSUPPORTED_FORMAT";
    opts.message = "Unsupported export format";
    return opts.status;
}

} // namespace core::controllers::exporting
