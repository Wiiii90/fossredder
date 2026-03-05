#include "ui/controllers/ExportController.h"

#include "ui/controllers/ControllerGuard.h"

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

#include <QtConcurrent/qtconcurrentrun.h>
#include <memory>

namespace ui {

namespace {

std::shared_ptr<const AppState> createExportSnapshot(const AppState& state)
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

}

ExportController::ExportController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
    connect(&exportWatcher_, &QFutureWatcher<core::controllers::exporting::ExportOptions>::finished, this, &ExportController::onExportFinished);
}

void ExportController::exportData(int format, const QString& path, bool includeFormulas, const QString& locale)
{
    if (!controllers::guard::ensureCore(core_, "ui::ExportController::exportData")) return;
    if (isRunning_) return;

    try {
        isRunning_ = true;
        emit stateChanged();

        auto csv = std::make_shared<core::controllers::exporting::CsvController>();
        auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
        core::controllers::exporting::ExportController exporter(xlsx, csv);

        core::controllers::exporting::ExportOptions opts;
        opts.outputPath = path.toStdString();
        opts.includeFormulas = includeFormulas;
        opts.locale = locale.toStdString();
        opts.stateSnapshot = createExportSnapshot(core_->state());
        opts.requestedFormat = (format == 0) ? core::controllers::exporting::ExportOptions::Format::Csv : core::controllers::exporting::ExportOptions::Format::Xlsx;

        exportFuture_ = QtConcurrent::run([exporter, opts]() mutable {
            exporter.exportData(opts);
            return opts;
        });
        exportWatcher_.setFuture(exportFuture_);
    } catch (...) {
        controllers::guard::reportException("ui::ExportController::exportData");
        isRunning_ = false;
        emit stateChanged();
        emit exportFinished(false);
    }
}

void ExportController::onExportFinished()
{
    bool success = false;
    try {
        const auto result = exportFuture_.result();
        success = (result.status == core::controllers::exporting::ExportOptions::Status::Ok);
        if (!success) {
            core::errors::report(core::errors::ErrorSeverity::Warning,
                                 result.errorCode.empty() ? core::errors::codes::GenericError : result.errorCode.c_str(),
                                 "ui::ExportController::onExportFinished",
                                 result.message.empty() ? "Export failed" : result.message);
        }
    } catch (...) {
        controllers::guard::reportException("ui::ExportController::onExportFinished");
        success = false;
    }

    isRunning_ = false;
    emit stateChanged();
    emit exportFinished(success);
}

}
