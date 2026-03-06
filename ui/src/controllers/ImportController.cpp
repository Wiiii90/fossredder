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
#include "ui/config/UiDefaults.h"
#include "ui/controllers/UiControllerContracts.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/import/ImportRunStore.h"
#include "ui/observability/UiTrace.h"

namespace ui {

namespace {

constexpr int kImportRunKeepCount = 20;
constexpr double kInitialImportProgress = 0.01;

}

ImportController::ImportController(std::shared_ptr<core::jobs::JobSystem> jobSystem, QObject* parent)
    : QObject(parent)
    , jobSystem_(std::move(jobSystem))
    , jobBridge_(jobSystem_)
    , runs_(this)
{
}

void ImportController::setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter)
{
    errorReporter_ = std::move(reporter);
}

void ImportController::addFiles(const QStringList& paths)
{
    QStringList cleaned;
    cleaned.reserve(paths.size());
    for (const auto& p : paths) {
        const auto t = p.trimmed();
        if (t.isEmpty()) continue;
        cleaned.push_back(t);
    }
    if (cleaned.isEmpty()) return;

    const bool hasSelection = !selectedFile_.trimmed().isEmpty();
    int startIndex = 0;
    if (!hasSelection) {
        setSelectedFile(cleaned.front());
        startIndex = 1;
    }

    for (int i = startIndex; i < cleaned.size(); ++i) queuedFiles_.push_back(cleaned[i]);
    emit stateChanged();
}

void ImportController::setSelectedFile(const QString& path)
{
    if (selectedFile_ == path) return;
    selectedFile_ = path;
    emit stateChanged();
}

void ImportController::resetStatus()
{
    if (isRunning_) return;
    phase_.clear();
    error_.clear();
    progress_ = 0.0;
    canceled_ = false;
    cancelClearsQueue_ = false;
    currentPage_ = 0;
    pageCount_ = 0;
    selectedFile_.clear();
    currentImportFile_.clear();
    queuedFiles_.clear();
    artifacts_.clear();
    emit stateChanged();
}

void ImportController::clearDraft()
{
    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }
    artifacts_.clear();
    if (!isRunning_ && queuedFiles_.isEmpty()) {
        selectedFile_.clear();
        currentImportFile_.clear();
    }
    emit stateChanged();
    if (!isRunning_ && !queuedFiles_.isEmpty()) {
        startNextQueuedImport();
    }
}

QByteArray ImportController::artifactBytes(const QString& key) const
{
    const auto it = artifacts_.find(key);
    if (it == artifacts_.end()) return {};
    return it.value();
}

QString ImportController::currentRunFile() const
{
    return currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_;
}

void ImportController::beginImportState(const QString& path)
{
    selectedFile_ = path;
    currentImportFile_ = path;

    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }

    artifacts_.clear();
    error_.clear();
    phase_ = controllers::contracts::importPhases::kStarting;
    progress_ = kInitialImportProgress;
    isRunning_ = true;
    canceled_ = false;
    emit stateChanged();
}

void ImportController::resetImportState()
{
    isRunning_ = false;
    progress_ = 0.0;
}

void ImportController::appendRun(const QString& now, const QString& status, const QString& message)
{
    runs_.addRun(now,
                 controllers::contracts::importRuns::kTypeStatement,
                 currentRunFile(),
                 status,
                 message);
}

void ImportController::handleImportCanceled(const QString& now)
{
    phase_ = controllers::contracts::importPhases::kCanceled;
    error_.clear();
    resetImportState();
    if (cancelClearsQueue_) queuedFiles_.clear();
    appendRun(now, controllers::contracts::importRuns::kStatusCanceled, {});
    selectedFile_.clear();
    currentImportFile_.clear();
    cancelClearsQueue_ = false;
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
    error_ = errorMessage;
    phase_ = controllers::contracts::importPhases::kFailed;
    resetImportState();
    queuedFiles_.clear();
    appendRun(now, controllers::contracts::importRuns::kStatusFailed, error_);
    currentImportFile_.clear();
    observability::reportFlow(core::errors::ErrorSeverity::Warning,
                              "ui::ImportController::onJobTerminal",
                              traceMessage,
                              {
                                  {"error", error_.toStdString()}
                              });
    emit stateChanged();
    emit importFailed(error_);
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

    artifacts_.clear();
    const auto arts = jobBridge_.takeArtifacts();
    for (const auto& kv : arts) {
        const QString k = QString::fromStdString(kv.first);
        const auto& v = kv.second;
        artifacts_.insert(k, v.empty() ? QByteArray() : QByteArray(reinterpret_cast<const char*>(v.data()), static_cast<int>(v.size())));
    }

    draft_ = importing::createStatementDraft(currentImportFile_, imported, this);

    phase_ = controllers::contracts::importPhases::kFinished;
    progress_ = 1.0;
    isRunning_ = false;
    appendRun(now, controllers::contracts::importRuns::kStatusSuccess, {});
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              "ui::ImportController::onJobTerminal",
                              "Import finished",
                              {
                                  {"status", controllers::contracts::importRuns::kStatusSuccess.toStdString()},
                                  {"artifactCount", std::to_string(artifacts_.size())}
                              });
    currentImportFile_.clear();
    emit stateChanged();
    emit importFinished();
    return true;
}

void ImportController::cancelImport()
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = false;
    phase_ = controllers::contracts::importPhases::kStopping;
    jobBridge_.cancelCurrent();
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              core::errors::codes::UiFlowImportCanceled,
                              "ui::ImportController::cancelImport",
                              "Import cancellation requested",
                              {
                                  {"file", currentImportFile_.toStdString()},
                                  {"queuedCount", std::to_string(queuedFiles_.size())}
                              });
    emit stateChanged();
}

void ImportController::cancelAllImports()
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = true;
    queuedFiles_.clear();
    phase_ = controllers::contracts::importPhases::kStopping;
    jobBridge_.cancelCurrent();
    observability::reportFlow(core::errors::ErrorSeverity::Info,
                              core::errors::codes::UiFlowImportCanceled,
                              "ui::ImportController::cancelAllImports",
                              "Cancel-all requested for import queue",
                              {
                                  {"file", currentImportFile_.toStdString()}
                              });
    emit stateChanged();
}

void ImportController::startNextQueuedImport()
{
    if (isRunning_) return;
    if (draft_) return;
    if (queuedFiles_.isEmpty()) return;

    const auto next = queuedFiles_.takeFirst();
    emit stateChanged();
    startImportForFile(next);
}

void ImportController::startStatementImport()
{
    if (isRunning_) return;
    if (draft_) return;

    const auto t = selectedFile_.trimmed();
    if (!t.isEmpty()) {
        selectedFile_.clear();
        emit stateChanged();
        startImportForFile(t);
        return;
    }

    startNextQueuedImport();
}

void ImportController::startImportForFile(const QString& path)
{
    if (isRunning_) return;
    if (!jobSystem_) {
        error_ = controllers::contracts::errors::kImportControllerUnavailable;
        queuedFiles_.clear();
        observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                  "ui::ImportController::startImportForFile",
                                  "Import start rejected: controller unavailable");
        emit stateChanged();
        emit importFailed(error_);
        return;
    }
    if (path.trimmed().isEmpty()) {
        error_ = controllers::contracts::errors::kNoFileSelected;
        queuedFiles_.clear();
        observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                  "ui::ImportController::startImportForFile",
                                  "Import start rejected: no file selected");
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    beginImportState(path);

    const QByteArray nativePath = QFile::encodeName(path);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    importing::cleanupOldImportRuns(kImportRunKeepCount);

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
                                  {"queuedCount", std::to_string(queuedFiles_.size())}
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
    if (!isRunning_ || canceled_) return;
    progress_ = p;
    if (!phase.isEmpty()) {
        phase_ = phase;

        static const QRegularExpression re(ui::config::kImportProgressPagePattern);
        const auto m = re.match(phase);
        if (m.hasMatch()) {
            bool ok1 = false;
            bool ok2 = false;
            const int cur = m.captured(1).toInt(&ok1);
            const int total = m.captured(2).toInt(&ok2);
            if (ok1 && ok2) {
                currentPage_ = cur;
                pageCount_ = total;
            }
        }
    }
    emit stateChanged();
}

void ImportController::onJobTerminal(int state, const QString& message)
{
    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);
    jobBridge_.clearSubscription([this](const char* origin, std::exception_ptr exception) {
        reportException(origin, exception);
    });

    const auto s = static_cast<core::jobs::JobState>(state);

    if (s == core::jobs::JobState::Canceled || canceled_) {
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
