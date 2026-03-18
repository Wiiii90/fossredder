/**
 * @file ui/include/ui/controllers/ExportController.h
 * @brief Declares the asynchronous export workflow controller exposed to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>
#include <functional>
#include <memory>

#include "ui/controllers/ControllerContracts.h"
#include "ui/export/ExportRunner.h"

namespace ui {

/**
 * @brief Coordinates asynchronous exports from a read-only state snapshot.
 */
class ExportController : public QObject {
    Q_OBJECT
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::AppState>()>;

    explicit ExportController(StateSnapshotProvider stateSnapshotProvider,
                              std::shared_ptr<ui::exporting::ExportRunner> runner,
                              QObject* parent = nullptr);

    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());

signals:
    void exportFinished(bool success);
    void exportFailed(const QString& error);

private slots:
    void onExportFinished();

private:
    ui::exporting::ExportRequest buildRequest(ui::controllers::contracts::ExportFormat format,
                                              const QString& path,
                                              bool includeFormulas,
                                              const QString& locale) const;
    void finishExport(bool success);
    std::shared_ptr<const core::domain::AppState> stateSnapshot() const;

    StateSnapshotProvider stateSnapshotProvider_;
    std::shared_ptr<ui::exporting::ExportRunner> runner_;
    QFuture<ui::exporting::ExportResult> exportFuture_;
    QFutureWatcher<ui::exporting::ExportResult> exportWatcher_;
    bool isRunning_ = false;
    QString lastError_;
};

} // namespace ui
