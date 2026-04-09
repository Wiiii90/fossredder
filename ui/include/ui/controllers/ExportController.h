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

#include "ui/bootstrap/QmlContracts.h"
#include "ui/export/ExportRunner.h"

namespace ui {

/**
 * @brief Coordinates asynchronous exports from a read-only state snapshot.
 */
class ExportController : public QObject {
    Q_OBJECT
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::AppState>()>;

    /** @brief Create an export controller backed by a snapshot provider and export runner. */
    explicit ExportController(StateSnapshotProvider stateSnapshotProvider,
                              std::shared_ptr<ui::exporting::ExportRunner> runner,
                              QObject* parent = nullptr);

    /** @brief Start an asynchronous export with the selected UI options.
     *  @param format Export format enum value
     *  @param path Output path
     *  @param includeFormulas Whether to include formulas
     *  @param locale Locale identifier
     */
    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());

signals:
    void exportFinished(bool success);
    void exportFailed(const QString& error);

private slots:
    /** @brief Finalize UI state once the asynchronous export completes. */
    void onExportFinished();

private:
    ui::exporting::ExportRequest buildRequest(ui::qml::contracts::ExportFormat format,
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
