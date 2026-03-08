#include "ui/controllers/ImportController.h"

#include <QDateTime>
#include <QFile>
#include <QMetaObject>
#include <QRegularExpression>

#include <exception>
#include <string>

#include "core/jobs/JobManager.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "ui/config/Defaults.h"
#include "ui/controllers/ControllerContracts.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/import/ImportRunStore.h"
#include "ui/observability/Trace.h"

namespace ui {

ImportController::ImportController(std::shared_ptr<core::jobs::JobSystem> jobSystem, QObject* parent)
    : QObject(parent)
    , runs_(this)
    , state_(runs_)
    , jobBridge_(std::move(jobSystem))
{
}

void ImportController::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    errorReporter_ = std::move(reporter);
}

void ImportController::addFiles(const QStringList& paths)
{
    if (state_.addFiles(paths)) emit stateChanged();
}

void ImportController::setSelectedFile(const QString& path)
{
    if (state_.setSelectedFile(path)) emit stateChanged();
}

void ImportController::resetStatus()
{
    if (state_.resetStatus()) emit stateChanged();
}

void ImportController::clearDraft()
{
    const bool shouldStartNext = state_.clearDraft();
    emit stateChanged();
    if (shouldStartNext) startNextQueuedImport();
}

void ImportController::handleImportCanceled(const QString& now)
{
    state_.recordCanceled(now);
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              "ui::ImportController::onJobTerminal",
                              "Import canceled",
                              {
                                  {"status", controllers::contracts::importRuns::kStatusCanceled.toStdString()}
                              });
    emit stateChanged();
    emit importCanceled();
}

void ImportController::handleImportFailed(const QString& now, const QString& errorMessage, const char* traceMessage)
{
    state_.recordFailed(now, errorMessage);
    observability::reportFlow(core::errors::ErrorSeverity::Warning,
                              "ui::ImportController::onJobTerminal",
                              traceMessage,
                              {
                                  {"error", state_.error().toStdString()}
                              });
    emit stateChanged();
    emit importFailed(state_.error());
}

bool ImportController::populateDraftFromResult(const QString& now)
{
    auto imported = jobBridge_.statementResult();
    if (!imported) {
        handleImportFailed(now,
                           controllers::contracts::errors::kImportFailed,
                           "Import failed: missing statement result");
        return false;
    }

    const auto artifacts = jobBridge_.takeArtifacts();
    if (!state_.populateDraft(now, imported, artifacts, this)) {
        handleImportFailed(now,
                           controllers::contracts::errors::kImportFailed,
                           "Import failed: unable to create statement draft");
        return false;
    }

    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              "ui::ImportController::onJobTerminal",
                              "Import finished",
                              {
                                  {"status", controllers::contracts::importRuns::kStatusSuccess.toStdString()},
                                  {"artifactCount", std::to_string(state_.artifactCount())}
                              });
    emit stateChanged();
    emit importFinished();
    return true;
}

void ImportController::cancelImport()
{
    if (!state_.isRunning()) return;
    state_.beginCancel(false);
    jobBridge_.cancelCurrent();
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              core::errors::codes::UiFlowImportCanceled,
                              "ui::ImportController::cancelImport",
                              "Import cancellation requested",
                              {
                                  {"file", state_.currentRunFile().toStdString()},
                                  {"queuedCount", std::to_string(state_.queuedFiles().size())}
                              });
    emit stateChanged();
}

void ImportController::cancelAllImports()
{
    if (!state_.isRunning()) return;
    state_.beginCancel(true);
    jobBridge_.cancelCurrent();
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              core::errors::codes::UiFlowImportCanceled,
                              "ui::ImportController::cancelAllImports",
                              "Cancel-all requested for import queue",
                              {
                                  {"file", state_.currentRunFile().toStdString()}
                              });
    emit stateChanged();
}

void ImportController::startNextQueuedImport()
{
    QString next;
    if (!state_.takeNextQueuedFile(next)) return;
    emit stateChanged();
    startImportForFile(next);
}

void ImportController::startStatementImport()
{
    if (state_.isRunning() || state_.draft()) return;

    const auto t = state_.takeSelectedFileForStart();
    if (!t.isEmpty()) {
        emit stateChanged();
        startImportForFile(t);
        return;
    }

    startNextQueuedImport();
}

void ImportController::startImportForFile(const QString& path)
{
    if (state_.isRunning()) return;
    if (!jobBridge_.isAvailable()) {
        state_.rejectStart(controllers::contracts::errors::kImportControllerUnavailable);
        observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                  "ui::ImportController::startImportForFile",
                                  "Import start rejected: controller unavailable");
        emit stateChanged();
        emit importFailed(state_.error());
        return;
    }
    if (path.trimmed().isEmpty()) {
        state_.rejectStart(controllers::contracts::errors::kNoFileSelected);
        observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                  "ui::ImportController::startImportForFile",
                                  "Import start rejected: no file selected");
        emit stateChanged();
        emit importFailed(state_.error());
        return;
    }

    state_.beginImport(path);
    emit stateChanged();

    const QByteArray nativePath = QFile::encodeName(path);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    importing::cleanupOldImportRuns(ui::config::kImportRunKeepCount);

    const auto runInfo = importing::createImportRunInfo();
    const QString runRootQ = runInfo.runRoot;
    const QString runIdPrefixQ = runInfo.runIdPrefix;
    const QByteArray runRootNative = QFile::encodeName(runRootQ);
    std::string runRoot(runRootNative.constData(), static_cast<size_t>(runRootNative.size()));

    const QByteArray runIdNative = runIdPrefixQ.toUtf8();
    std::string runIdPrefix(runIdNative.constData(), static_cast<size_t>(runIdNative.size()));

    core::jobs::ImportStatementJobSpec spec;
    spec.sourcePath = p;
    spec.runRoot = runRoot;
    spec.runIdPrefix = runIdPrefix;

    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              core::errors::codes::UiFlowImportStarted,
                              "ui::ImportController::startImportForFile",
                              "Import started",
                              {
                                  {"file", path.toStdString()},
                                  {"runRoot", runRootQ.toStdString()},
                                  {"queuedCount", std::to_string(state_.queuedFiles().size())}
                              });

    const bool started = jobBridge_.startStatementImport(spec, [this](const core::jobs::JobEvent& ev) {
        double p = ev.progress;
        if (p < 0.0) p = 0.0;
        if (p > 1.0) p = 1.0;
        const QString phase = QString::fromStdString(ev.message);

        QMetaObject::invokeMethod(this, [this, p, phase, ev]() {
            updateProgress(p, phase);
            if (ev.state == core::jobs::JobState::Finished || ev.state == core::jobs::JobState::Failed || ev.state == core::jobs::JobState::Canceled) {
                onJobTerminal(static_cast<int>(ev.state), QString::fromStdString(ev.message));
            }
        }, Qt::QueuedConnection);
    }, [this](const char* origin, std::exception_ptr exception) {
        reportException(origin, exception);
    });

    if (!started) {
        handleImportFailed(QDateTime::currentDateTime().toString(Qt::ISODate),
                           controllers::contracts::errors::kImportFailed,
                           "Import failed: unable to start job");
    }
}

void ImportController::updateProgress(double p, const QString& phase)
{
    static const QRegularExpression re(ui::config::kImportProgressPagePattern);
    state_.updateProgress(p, phase, re);
    emit stateChanged();
}

void ImportController::onJobTerminal(int state, const QString& message)
{
    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);
    jobBridge_.clearSubscription([this](const char* origin, std::exception_ptr exception) {
        reportException(origin, exception);
    });

    const auto s = static_cast<core::jobs::JobState>(state);

    if (s == core::jobs::JobState::Canceled || state_.cancelRequested()) {
        handleImportCanceled(now);
        return;
    }

    if (s == core::jobs::JobState::Failed) {
        handleImportFailed(now,
                           message.isEmpty() ? controllers::contracts::errors::kImportFailed : message,
                           "Import failed");
        return;
    }

    populateDraftFromResult(now);
}

void ImportController::reportException(const char* origin, std::exception_ptr exception) const
{
    if (errorReporter_) {
        errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin, exception);
    }
}

}
