/**
 * @file core/include/core/application/export/XlsxExporter.h
 * @brief Declares XLSX export for the property/contract-type matrix.
 */

#pragma once

#include "core/application/export/ExportRequest.h"
#include "core/application/export/ExportResult.h"

namespace core::ports::xlsx_writer {
class IXlsxWriter;
}

namespace core::application::exporting {

/**
 * @brief Exports the property/contract matrix to XLSX.
 */
class XlsxExporter {
public:
    explicit XlsxExporter(std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> writer = {});

    /**
     * @brief Exports the property/contract matrix to an XLSX file.
     * @param request Export request describing output location and source snapshot.
     * @return Export result describing success or failure.
     */
    ExportResult exportData(const ExportRequest& request) const;

private:
    std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> writer_;
};

} // namespace core::application::exporting
