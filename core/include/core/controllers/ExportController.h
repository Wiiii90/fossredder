/**
 * @file core/include/core/controllers/ExportController.h
 * @brief Declares the export entry point used by the application shell.
 */

#pragma once

#include "core/export/ExportRequest.h"
#include "core/export/ExportResult.h"

namespace core::controllers::exporting {

class ExportController {
public:
    ExportResult exportData(const ExportRequest& request) const;
};

} // namespace core::controllers::exporting
