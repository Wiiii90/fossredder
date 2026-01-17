#pragma once

#include <string>
#include "core/models/AppState.h"

namespace core::controllers::exporting {

struct ExportOptions {
    enum class Format {
        Csv,
        Xlsx
    } format = Format::Xlsx;

    // Output path (file)
    std::string outputPath;

    // If true, include formulas (for XLSX path). For CSV this is ignored.
    bool includeFormulas = true;

    // Locale string, e.g. "de-DE" or "en-US" (may affect decimal separator/format)
    std::string locale;

    // Pointer to AppState snapshot to export from. If null, export will fail.
    const AppState* state = nullptr;
};

} // namespace core::controllers::exporting
