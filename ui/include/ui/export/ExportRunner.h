#pragma once

#include <memory>

#include <QString>

#include "core/export/ExportOptions.h"
#include "core/models/AppState.h"

namespace ui::exporting {

struct ExportRequest {
    int format = 0;
    QString path;
    bool includeFormulas = true;
    QString locale;
};

class ExportRunner {
public:
    core::controllers::exporting::ExportOptions createOptions(const AppState& state, const ExportRequest& request) const;
    core::controllers::exporting::ExportOptions run(core::controllers::exporting::ExportOptions options) const;

private:
    core::controllers::exporting::ExportOptions::Format toExportFormat(int format) const;
    std::shared_ptr<const AppState> createSnapshot(const AppState& state) const;
};

}
