/**
 * @file core/include/core/export/CsvExporter.h
 * @brief Declares CSV export for the property/contract-type matrix.
 */

#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::exporting {

/**
 * @brief Writes the export matrix to a CSV file.
 */
class CsvExporter {
public:
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::exporting
