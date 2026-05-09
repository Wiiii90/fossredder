/**
 * @file ui/include/ui/controllers/ExportController.h
 * @brief Declares the asynchronous export workflow controller exposed to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFuture>
#include <QFutureWatcher>
#include <qqmlintegration.h>
#include <functional>
#include <memory>

#include "core/application/export/ExportLog.h"
#include "core/ports/presenters/IExportPresenter.h"
#include "ui/models/ExportRunList.h"

#include "ui/bootstrap/QmlContracts.h"
#include "ui/export/ExportRunner.h"

namespace ui {

/**
 * @brief Coordinates asynchronous exports from a read-only state snapshot.
 */
class ExportController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ExportController)
    QML_UNCREATABLE("ExportController is provided by the application context")
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
    Q_PROPERTY(QString phase READ phase NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(int currentMode READ currentMode NOTIFY stateChanged)
    Q_PROPERTY(ExportRunList* runs READ runs CONSTANT)
    Q_PROPERTY(bool hasPrevRun READ hasPrevRun NOTIFY stateChanged)
    Q_PROPERTY(bool hasNextRun READ hasNextRun NOTIFY stateChanged)
    Q_PROPERTY(int currentRunIndex READ currentRunIndex NOTIFY stateChanged)
public:
    enum Mode {
        CreateMode = 0,
        ProgressMode = 1
    };
    Q_ENUM(Mode)

    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::WorkspaceState>()>;
    using ExportLogsStore = std::function<void(const std::vector<core::domain::ExportLog>&)>;

    /** @brief Create an export controller backed by a snapshot provider and export runner. */
    explicit ExportController(StateSnapshotProvider stateSnapshotProvider,
                              std::shared_ptr<ui::exporting::ExportRunner> runner,
                              std::shared_ptr<core::ports::presenters::IExportPresenter> exportPresenter = {},
                              QObject* parent = nullptr);

    void setExportLogsStore(ExportLogsStore store);

    bool isRunning() const noexcept { return isRunning_; }
    bool isPaused() const noexcept { return isPaused_; }
    double progress() const noexcept { return progress_; }
    QString phase() const { return phase_; }
    QString error() const { return lastError_; }
    int currentMode() const noexcept;
    ExportRunList* runs() noexcept;
    bool hasPrevRun() const;
    bool hasNextRun() const;
    int currentRunIndex() const;

    /** @brief Start an asynchronous export with the selected UI options.
     *  @param format Export format enum value
     *  @param path Output path
     *  @param includeFormulas Whether to include formulas
     *  @param locale Locale identifier
     */
    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());
    Q_INVOKABLE void exportDataWithPayload(int format,
                                           const QString& path,
                                           bool includeFormulas,
                                           const QString& locale,
                                           const QString& payload,
                                           int totalSteps = 1);
    Q_INVOKABLE void activateRunAt(int index);
    Q_INVOKABLE void removeRunAt(int index);
    Q_INVOKABLE void clearRuns();
    Q_INVOKABLE bool openRunLocationAt(int index);
    Q_INVOKABLE bool openPrevRun();
    Q_INVOKABLE bool openNextRun();
    Q_INVOKABLE void clearActiveRun();
    Q_INVOKABLE void cancelExport();
    Q_INVOKABLE void togglePause();

signals:
    void stateChanged();
    void exportFinished(bool success);
    void exportFailed(const QString& error);
    void runActivated(const QString& payload);

private slots:
    /** @brief Finalize UI state once the asynchronous export completes. */
    void onExportFinished();

private:
    ui::exporting::ExportRequest buildRequest(ui::qml::contracts::ExportFormat format,
                                              const QString& path,
                                              bool includeFormulas,
                                              const QString& locale) const;
    void restoreRunsFromSnapshot() const;
    void persistRuns();
    QString generateLogId() const;
    QString currentTimestamp() const;
    ExportRunRow upsertRunById(const QString& logId,
                               const QString& path,
                               const QString& status,
                               const QString& message,
                               const QString& payload);
    void finishExport(bool success);
    std::shared_ptr<const core::domain::WorkspaceState> stateSnapshot() const;

    StateSnapshotProvider stateSnapshotProvider_;
    ExportLogsStore exportLogsStore_;
    std::shared_ptr<ui::exporting::ExportRunner> runner_;
    std::shared_ptr<core::ports::presenters::IExportPresenter> exportPresenter_;
    std::unique_ptr<ExportRunList> runs_;
    QFuture<ui::exporting::ExportResult> exportFuture_;
    QFutureWatcher<ui::exporting::ExportResult> exportWatcher_;
    bool isRunning_ = false;
    bool isPaused_ = false;
    bool cancelRequested_ = false;
    double progress_ = 0.0;
    QString phase_;
    int totalSteps_ = 1;
    int completedSteps_ = 0;
    QString pendingPayload_;
    QString activeRunLogId_;
    QString activeRunPath_;
    QString lastError_;
};

} // namespace ui
