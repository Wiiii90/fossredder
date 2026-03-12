/**
 * @file core/include/core/controllers/CsvController.h
 * @brief CSV export controller for the property/contract-type matrix.
 */
#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::controllers::exporting {

class CsvController {
public:
    CsvController() = default;
    ExportResult exportData(const ExportRequest& request);
};

} // namespace core::controllers::exporting
