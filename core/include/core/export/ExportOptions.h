#pragma once

#include <memory>
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

    // Immutable AppState snapshot to export from. If null, export will fail.
    std::shared_ptr<const AppState> stateSnapshot;
};

} // namespace core::controllers::exporting
