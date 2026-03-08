#include "ui/export/ExportRunner.h"

#include <memory>
#include <string>

#include "core/controllers/CsvController.h"
#include "core/controllers/ExportController.h"
#include "core/controllers/XlsxController.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "ui/controllers/ControllerContracts.h"
#include "ui/observability/Trace.h"

namespace ui::exporting {

core::controllers::exporting::ExportOptions::Format ExportRunner::toExportFormat(int format) const
{
    using UiExportFormat = ui::controllers::contracts::ExportFormat;
    switch (static_cast<UiExportFormat>(format)) {
    case UiExportFormat::Csv:
        return core::controllers::exporting::ExportOptions::Format::Csv;
    case UiExportFormat::Xlsx:
        return core::controllers::exporting::ExportOptions::Format::Xlsx;
    default:
        ui::observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::UiFlowExportFallback,
                                      "ui::exporting::ExportRunner::toExportFormat",
                                      "Unknown export format, fallback to CSV",
                                      {
                                          {"format", std::to_string(format)}
                                      });
        return core::controllers::exporting::ExportOptions::Format::Csv;
    }
}

std::shared_ptr<const AppState> ExportRunner::createSnapshot(const AppState& state) const
{
    auto snapshot = std::make_shared<AppState>();

    snapshot->properties.reserve(state.properties.size());
    for (const auto& item : state.properties) {
        if (!item) continue;
        snapshot->properties.push_back(std::make_shared<Property>(*item));
    }

    snapshot->actors.reserve(state.actors.size());
    for (const auto& item : state.actors) {
        if (!item) continue;
        snapshot->actors.push_back(std::make_shared<Actor>(*item));
    }

    snapshot->contracts.reserve(state.contracts.size());
    for (const auto& item : state.contracts) {
        if (!item) continue;
        snapshot->contracts.push_back(std::make_shared<Contract>(*item));
    }

    snapshot->statements.reserve(state.statements.size());
    for (const auto& item : state.statements) {
        if (!item) continue;
        snapshot->statements.push_back(std::make_shared<Statement>(*item));
    }

    snapshot->transactions.reserve(state.transactions.size());
    for (const auto& item : state.transactions) {
        if (!item) continue;
        snapshot->transactions.push_back(std::make_shared<Transaction>(*item));
    }

    snapshot->analyses.reserve(state.analyses.size());
    for (const auto& item : state.analyses) {
        if (!item) continue;
        snapshot->analyses.push_back(std::make_shared<Analysis>(*item));
    }

    snapshot->annuals.reserve(state.annuals.size());
    for (const auto& item : state.annuals) {
        if (!item) continue;
        snapshot->annuals.push_back(std::make_shared<Annual>(*item));
    }

    return snapshot;
}

core::controllers::exporting::ExportOptions ExportRunner::createOptions(const AppState& state, const ExportRequest& request) const
{
    core::controllers::exporting::ExportOptions options;
    options.outputPath = request.path.toStdString();
    options.includeFormulas = request.includeFormulas;
    options.locale = request.locale.toStdString();
    options.stateSnapshot = createSnapshot(state);
    options.requestedFormat = toExportFormat(request.format);
    return options;
}

core::controllers::exporting::ExportOptions ExportRunner::run(core::controllers::exporting::ExportOptions options) const
{
    auto csv = std::make_shared<core::controllers::exporting::CsvController>();
    auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
    core::controllers::exporting::ExportController exporter(xlsx, csv);
    exporter.exportData(options);
    return options;
}

}
