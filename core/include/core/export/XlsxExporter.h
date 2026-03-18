/**
 * @file core/include/core/export/XlsxExporter.h
 * @brief Declares XLSX export for the property/contract-type matrix.
 */

#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::exporting {

/**
 * @brief Writes the export matrix to an XLSX file.
 */
class XlsxExporter {
public:
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::exporting
