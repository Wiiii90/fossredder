/**
 * @file core/include/core/controllers/XlsxController.h
 * @brief XLSX export controller for the property/contract-type matrix.
 */
#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::controllers::exporting {

class XlsxController {
public:
    XlsxController() = default;
    ExportResult exportData(const ExportRequest& request);
};

} // namespace core::controllers::exporting
