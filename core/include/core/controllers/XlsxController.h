#pragma once

#include "core/export/ExportOptions.h"

namespace core::controllers::exporting {

class XlsxController {
public:
    XlsxController() = default;
    bool exportData(const ExportOptions& opts);
};

} // namespace core::controllers::exporting
