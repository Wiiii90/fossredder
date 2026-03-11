#include "core/controllers/ExportController.h"

#include "core/constants/CoreDefaults.h"
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
            opts.errorCode = std::string(core::constants::exportFlow::kErrorCsvControllerMissing);
            opts.message = std::string(core::constants::exportFlow::kMessageCsvControllerMissing);
            return opts.status;
        }
        return csv_->exportData(opts);
    }
    if (opts.requestedFormat == ExportOptions::Format::Xlsx) {
        if (!xlsx_) {
            opts.status = ExportOptions::Status::InternalError;
            opts.errorCode = std::string(core::constants::exportFlow::kErrorXlsxControllerMissing);
            opts.message = std::string(core::constants::exportFlow::kMessageXlsxControllerMissing);
            return opts.status;
        }
        return xlsx_->exportData(opts);
    }

    opts.status = ExportOptions::Status::UnsupportedFormat;
    opts.errorCode = std::string(core::constants::exportFlow::kErrorUnsupportedFormat);
    opts.message = std::string(core::constants::exportFlow::kMessageUnsupportedFormat);
    return opts.status;
}

} // namespace core::controllers::exporting
