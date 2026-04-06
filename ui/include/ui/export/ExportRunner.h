/**
 * @file ui/include/ui/export/ExportRunner.h
 * @brief Declares the UI-facing export execution wrapper and payload types.
 */

#pragma once

#include <functional>
#include <memory>

#include <QString>

#include "core/models/AppState.h"
#include "ui/bootstrap/QmlContracts.h"

namespace ui::exporting {

/** @brief Captures the user-selected export options for a single export run. */
struct ExportRequest {
  ui::qml::contracts::ExportFormat format = ui::qml::contracts::ExportFormat::Csv;
  QString path;
  bool includeFormulas = true;
  QString locale;
};

/** @brief Represents the outcome of an export execution. */
struct ExportResult {
  bool success = false;
  QString errorCode;
  QString message;
};

/**
 * @brief Executes exports against a captured application state snapshot.
 */
class ExportRunner {
public:
  using ExecuteExportFn = std::function<ExportResult(
      std::shared_ptr<const core::domain::AppState>, const ExportRequest &)>;

  explicit ExportRunner(ExecuteExportFn execute = {});

  ExportResult run(std::shared_ptr<const core::domain::AppState> state,
                   const ExportRequest &request) const;
  ExportResult run(const core::domain::AppState &state,
                   const ExportRequest &request) const;

  ExecuteExportFn execute_;
};

} // namespace ui::exporting
