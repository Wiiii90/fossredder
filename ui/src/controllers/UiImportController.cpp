#include "ui/controllers/UiImportController.h"
#include "ui/controllers/UiExportController.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QMetaObject>
#include <QRegularExpression>

#include "core/jobs/JobSystem.h"
#include "core/jobs/JobManager.h"
#include "core/jobs/JobTypes.h"
#include "ui/controllers/UiDomainController.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

UiImportController::UiImportController(std::shared_ptr<core::jobs::JobSystem> jobSystem, QObject* parent)
    : QObject(parent), jobSystem_(std::move(jobSystem))
    , runs_(this)
{
}

void UiImportController::setDomainController(UiDomainController* domain)
{
    domain_ = domain;
}

void UiImportController::setSelectedFile(const QString& path)
{
    if (selectedFile_ == path) return;
    selectedFile_ = path;
    emit stateChanged();
}

QStringList UiImportController::profiles() const
{
    return { QStringLiteral("Default"), QStringLiteral("High DPI (300)"), QStringLiteral("Fast (200)") };
}

void UiImportController::setSelectedProfile(const QString& profile)
{
    if (selectedProfile_ == profile) return;
    selectedProfile_ = profile;
    emit stateChanged();
}

void UiImportController::resetStatus()
{
    phase_.clear();
    error_.clear();
    progress_ = 0.0;
    canceled_ = false;
    currentPage_ = 0;
    pageCount_ = 0;
    emit stateChanged();
}

void UiImportController::clearDraft()
{
    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }
    artifacts_.clear();
    emit stateChanged();
}

QByteArray UiImportController::artifactBytes(const QString& key) const
{
    const auto it = artifacts_.find(key);
    if (it == artifacts_.end()) return {};
    return it.value();
}

void UiImportController::cancelImport()
{
    if (!isRunning_) return;
    canceled_ = true;
    phase_ = QStringLiteral("Stopping...");
    if (jobSystem_ && !currentJobId_.isEmpty()) jobSystem_->manager().cancel(currentJobId_.toStdString());
    emit stateChanged();
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

    // match: <ts>_import_<n>
    const QStringList dirs = d.entryList(QStringList() << "*_import_*", QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    if (dirs.size() <= keep) return;

    for (int i = keep; i < dirs.size(); ++i) {
        QDir rm(d.filePath(dirs[i]));
        rm.removeRecursively();
    }
}

void UiImportController::startStatementImport()
{
    if (isRunning_) return;
    if (!jobSystem_) {
        error_ = QStringLiteral("Import controller not available");
        emit stateChanged();
        emit importFailed(error_);
        return;
    }
    if (selectedFile_.trimmed().isEmpty()) {
        error_ = QStringLiteral("No file selected");
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    clearDraft();

    error_.clear();
    phase_ = QStringLiteral("Starting import...");
    progress_ = 0.01;
    isRunning_ = true;
    canceled_ = false;
    emit stateChanged();

    const QByteArray nativePath = QFile::encodeName(selectedFile_);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

    cleanupOldImportRuns(20);

    QString runIdPrefixQ;
    const QString runRootQ = makeImportRunRootDir(&runIdPrefixQ);
    const QByteArray runRootNative = QFile::encodeName(runRootQ);
    std::string runRoot(runRootNative.constData(), static_cast<size_t>(runRootNative.size()));

    const QByteArray runIdNative = runIdPrefixQ.toUtf8();
    std::string runIdPrefix(runIdNative.constData(), static_cast<size_t>(runIdNative.size()));

    if (currentSubId_ != 0 && !currentJobId_.isEmpty()) {
        try { jobSystem_->manager().unsubscribe(currentJobId_.toStdString(), currentSubId_); } catch (...) {}
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

void UiImportController::updateProgress(double p, const QString& phase)
{
    // ignore updates when cancelled or not running
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

void UiImportController::onJobTerminal(int state, const QString& message)
{
    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (jobSystem_ && currentSubId_ != 0 && !currentJobId_.isEmpty()) {
        try { jobSystem_->manager().unsubscribe(currentJobId_.toStdString(), currentSubId_); } catch (...) {}
    }
    currentSubId_ = 0;

    const auto s = static_cast<core::jobs::JobState>(state);

    if (s == core::jobs::JobState::Canceled || canceled_) {
        phase_ = QStringLiteral("Import canceled");
        error_.clear();
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Canceled"), QStringLiteral(""));
        emit stateChanged();
        emit importFailed(QStringLiteral("Canceled"));
        return;
    }

    if (s == core::jobs::JobState::Failed) {
        error_ = message.isEmpty() ? QStringLiteral("Import failed") : message;
        phase_ = QStringLiteral("Import failed");
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Failed"), error_);
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    auto imported = jobSystem_ && !currentJobId_.isEmpty() ? jobSystem_->manager().statementResult(currentJobId_.toStdString()) : nullptr;
    if (!imported) {
        error_ = QStringLiteral("Import failed");
        phase_ = QStringLiteral("Import failed");
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Failed"), error_);
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    // Pull in-memory artifacts (proof images etc.) for the draft preview.
    artifacts_.clear();
    if (jobSystem_ && !currentJobId_.isEmpty()) {
        const auto arts = jobSystem_->manager().statementArtifacts(currentJobId_.toStdString());
        for (const auto& kv : arts) {
            const QString k = QString::fromStdString(kv.first);
            const auto& v = kv.second;
            artifacts_.insert(k, v.empty() ? QByteArray() : QByteArray(reinterpret_cast<const char*>(v.data()), static_cast<int>(v.size())));
        }
    }

    draft_ = new StatementDraft(this);
    draft_->setName(QFileInfo(selectedFile_).baseName());

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

    phase_ = QStringLiteral("Import finished");
    progress_ = 1.0;
    isRunning_ = false;
    runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Success"), QStringLiteral(""));
    emit stateChanged();
    emit importFinished();
}
