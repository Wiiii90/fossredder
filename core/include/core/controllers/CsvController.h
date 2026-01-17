#pragma once

#include "core/export/ExportOptions.h"

namespace core::controllers::exporting {

class CsvController {
public:
    CsvController() = default;
    bool exportData(const ExportOptions& opts);
};

} // namespace core::controllers::exporting
