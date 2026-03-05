#include "ui/controllers/ImportController.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QStandardPaths>

#include <exception>

#include "core/jobs/JobManager.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "ui/models/TransactionDraft.h"

namespace ui {

namespace {

constexpr int kImportRunKeepCount = 20;
constexpr double kInitialImportProgress = 0.01;

const auto kImportTypeStatement = QStringLiteral("Statement");
const auto kStatusCanceled = QStringLiteral("Canceled");
const auto kStatusFailed = QStringLiteral("Failed");
const auto kStatusSuccess = QStringLiteral("Success");

const auto kPhaseStopping = QStringLiteral("Stopping...");
const auto kPhaseStartingImport = QStringLiteral("Starting import...");
const auto kPhaseImportCanceled = QStringLiteral("Import canceled");
const auto kPhaseImportFailed = QStringLiteral("Import failed");
const auto kPhaseImportFinished = QStringLiteral("Import finished");

const auto kErrorImportControllerUnavailable = QStringLiteral("Import controller not available");
const auto kErrorNoFileSelected = QStringLiteral("No file selected");
const auto kErrorImportFailed = QStringLiteral("Import failed");

}

ImportController::ImportController(std::shared_ptr<core::jobs::JobSystem> jobSystem, QObject* parent)
    : QObject(parent)
    , jobSystem_(std::move(jobSystem))
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

void ImportController::cancelImport()
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = false;
    phase_ = kPhaseStopping;
    if (jobSystem_ && !currentJobId_.isEmpty()) jobSystem_->manager().cancel(currentJobId_.toStdString());
    emit stateChanged();
}

void ImportController::cancelAllImports()
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = true;
    queuedFiles_.clear();
    phase_ = kPhaseStopping;
    if (jobSystem_ && !currentJobId_.isEmpty()) jobSystem_->manager().cancel(currentJobId_.toStdString());
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

static QString makeImportRunRootDir(QString* outRunIdPrefix)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d(base);
    d.mkpath(".");

    const QString ts = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    if (outRunIdPrefix) *outRunIdPrefix = ts;

    int n = 1;
    while (true) {
        const QString name = QStringLiteral("%1_import_%2").arg(ts).arg(n);
        if (!d.exists(name)) {
            d.mkpath(name);
            return d.filePath(name);
        }
        ++n;
    }
}

static void cleanupOldImportRuns(int keep)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d(base);
    if (!d.exists()) return;

    const QStringList dirs = d.entryList(QStringList() << "*_import_*", QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    if (dirs.size() <= keep) return;

    for (int i = keep; i < dirs.size(); ++i) {
        QDir rm(d.filePath(dirs[i]));
        rm.removeRecursively();
    }
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
        error_ = kErrorImportControllerUnavailable;
        queuedFiles_.clear();
        emit stateChanged();
        emit importFailed(error_);
        return;
    }
    if (path.trimmed().isEmpty()) {
        error_ = kErrorNoFileSelected;
        queuedFiles_.clear();
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    selectedFile_ = path;
    currentImportFile_ = path;

    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }
    artifacts_.clear();

    error_.clear();
    phase_ = kPhaseStartingImport;
    progress_ = kInitialImportProgress;
    isRunning_ = true;
    canceled_ = false;
    emit stateChanged();

    const QByteArray nativePath = QFile::encodeName(path);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    cleanupOldImportRuns(kImportRunKeepCount);

    QString runIdPrefixQ;
    const QString runRootQ = makeImportRunRootDir(&runIdPrefixQ);
    const QByteArray runRootNative = QFile::encodeName(runRootQ);
    std::string runRoot(runRootNative.constData(), static_cast<size_t>(runRootNative.size()));

    const QByteArray runIdNative = runIdPrefixQ.toUtf8();
    std::string runIdPrefix(runIdNative.constData(), static_cast<size_t>(runIdNative.size()));

    if (currentSubId_ != 0 && !currentJobId_.isEmpty()) {
        try {
            jobSystem_->manager().unsubscribe(currentJobId_.toStdString(), currentSubId_);
        } catch (...) {
            reportException("ui::ImportController::startImportForFile::unsubscribe", std::current_exception());
        }
    }
    currentSubId_ = 0;
    currentJobId_.clear();

    core::jobs::ImportStatementJobSpec spec;
    spec.sourcePath = p;
    spec.runRoot = runRoot;
    spec.runIdPrefix = runIdPrefix;

    const auto jobId = jobSystem_->startImportStatement(spec);
    currentJobId_ = QString::fromStdString(jobId);

    currentSubId_ = jobSystem_->manager().subscribe(jobId, [this](const core::jobs::JobEvent& ev) {
        if (currentJobId_.isEmpty()) return;
        if (QString::fromStdString(ev.jobId) != currentJobId_) return;

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
    });
}

void ImportController::updateProgress(double p, const QString& phase)
{
    if (!isRunning_ || canceled_) return;
    progress_ = p;
    if (!phase.isEmpty()) {
        phase_ = phase;

        static const QRegularExpression re(QStringLiteral("\\[(\\d+)\\s*/\\s*(\\d+)\\]"));
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

    if (jobSystem_ && currentSubId_ != 0 && !currentJobId_.isEmpty()) {
        try {
            jobSystem_->manager().unsubscribe(currentJobId_.toStdString(), currentSubId_);
        } catch (...) {
            reportException("ui::ImportController::onJobTerminal::unsubscribe", std::current_exception());
        }
    }
    currentSubId_ = 0;

    const auto s = static_cast<core::jobs::JobState>(state);

    if (s == core::jobs::JobState::Canceled || canceled_) {
        phase_ = kPhaseImportCanceled;
        error_.clear();
        isRunning_ = false;
        progress_ = 0.0;
        if (cancelClearsQueue_) queuedFiles_.clear();
        runs_.addRun(now, kImportTypeStatement, currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_, kStatusCanceled, QStringLiteral(""));
        selectedFile_.clear();
        currentImportFile_.clear();
        cancelClearsQueue_ = false;
        emit stateChanged();
        emit importFailed(kStatusCanceled);
        return;
    }

    if (s == core::jobs::JobState::Failed) {
        error_ = message.isEmpty() ? kErrorImportFailed : message;
        phase_ = kPhaseImportFailed;
        isRunning_ = false;
        progress_ = 0.0;
        queuedFiles_.clear();
        runs_.addRun(now, kImportTypeStatement, currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_, kStatusFailed, error_);
        currentImportFile_.clear();
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    auto imported = jobSystem_ && !currentJobId_.isEmpty() ? jobSystem_->manager().statementResult(currentJobId_.toStdString()) : nullptr;
    if (!imported) {
        error_ = kErrorImportFailed;
        phase_ = kPhaseImportFailed;
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, kImportTypeStatement, currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_, kStatusFailed, error_);
        currentImportFile_.clear();
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    artifacts_.clear();
    if (jobSystem_ && !currentJobId_.isEmpty()) {
        const auto arts = jobSystem_->manager().takeStatementArtifacts(currentJobId_.toStdString());
        for (const auto& kv : arts) {
            const QString k = QString::fromStdString(kv.first);
            const auto& v = kv.second;
            artifacts_.insert(k, v.empty() ? QByteArray() : QByteArray(reinterpret_cast<const char*>(v.data()), static_cast<int>(v.size())));
        }
    }

    draft_ = new StatementDraft(this);
    draft_->setName(QFileInfo(currentImportFile_).baseName());

    std::vector<TransactionDraft> txs;
    txs.reserve(imported->transactions.size());
    for (const auto& txptr : imported->transactions) {
        if (!txptr) continue;
        TransactionDraft d;
        d.name = QString::fromStdString(txptr->name);
        d.bookingDate = QString::fromStdString(txptr->bookingDate);
        d.valuta = QString::fromStdString(txptr->valuta);
        d.amount = txptr->amount;
        d.description = QString::fromStdString(txptr->description);
        d.actorId = QString();
        d.actorProposal = QString::fromStdString(txptr->actorProposal);
        d.metadata = QString::fromStdString(txptr->metadata);
        d.proofImagePath = QString::fromStdString(txptr->proofImagePath);
        d.status = static_cast<int>(Transaction::Status::Unverified);
        txs.push_back(std::move(d));
    }
    draft_->setDrafts(std::move(txs));

    phase_ = kPhaseImportFinished;
    progress_ = 1.0;
    isRunning_ = false;
    runs_.addRun(now, kImportTypeStatement, currentImportFile_, kStatusSuccess, QStringLiteral(""));
    currentImportFile_.clear();
    emit stateChanged();
    emit importFinished();
}

void ImportController::reportException(const char* origin, std::exception_ptr exception) const
{
    if (errorReporter_) {
        errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin, exception);
    }
}

}
