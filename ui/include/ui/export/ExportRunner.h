#pragma once

#include <functional>
#include <memory>

#include <QString>

#include "core/models/AppState.h"
#include "ui/controllers/ControllerContracts.h"

namespace ui::exporting {

struct ExportRequest {
  ui::controllers::contracts::ExportFormat format =
      ui::controllers::contracts::ExportFormat::Csv;
  QString path;
  bool includeFormulas = true;
  QString locale;
};

struct ExportResult {
  bool success = false;
  QString errorCode;
  QString message;
};

class ExportRunner {
public:
  using ExecuteExportFn = std::function<ExportResult(
      std::shared_ptr<const AppState>, const ExportRequest &)>;

  explicit ExportRunner(ExecuteExportFn execute = {});

  ExportResult run(std::shared_ptr<const AppState> state,
                   const ExportRequest &request) const;
  ExportResult run(const AppState &state, const ExportRequest &request) const;

  ExecuteExportFn execute_;
};

} // namespace ui::exporting
