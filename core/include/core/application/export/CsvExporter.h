/**
 * @file core/include/core/application/export/CsvExporter.h
 * @brief Declares CSV export for the property/contract-type matrix.
 */

#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

namespace core::application::exporting {

/**
 * @brief Exports the property/contract matrix to CSV.
 */
class CsvExporter {
public:
    /**
     * @brief Exports the property/contract matrix to a CSV file.
     * @param request Export request describing output location and source snapshot.
     * @return Export result describing success or failure.
     */
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::application::exporting
