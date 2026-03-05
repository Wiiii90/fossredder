#pragma once

#include "core/export/ExportOptions.h"

namespace core::controllers::exporting {

class XlsxController {
public:
    XlsxController() = default;
    ExportOptions::Status exportData(ExportOptions& opts);
};

} // namespace core::controllers::exporting
