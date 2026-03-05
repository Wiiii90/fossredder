#pragma once

#include "core/export/ExportOptions.h"

namespace core::controllers::exporting {

class CsvController {
public:
    CsvController() = default;
    ExportOptions::Status exportData(ExportOptions& opts);
};

} // namespace core::controllers::exporting
