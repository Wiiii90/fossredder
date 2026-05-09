#pragma once

#include "core/application/export/ExportTypes.h"

#include <string>

namespace core::ports::presenters {

struct ExportPresentation {
    bool success = false;
    core::application::exporting::ExportStatus status = core::application::exporting::ExportStatus::InternalError;
    core::application::exporting::ExportFormat actualFormat = core::application::exporting::ExportFormat::Xlsx;
    std::string resolvedOutputPath;
    std::string errorCode;
    std::string message;
};

class IExportPresenter {
public:
    virtual ~IExportPresenter() = default;

    virtual ExportPresentation present(const ExportPresentation& result) const = 0;
};

} // namespace core::ports::presenters
