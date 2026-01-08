#include "ui/controllers/UiImportController.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

#include "core/controllers/ImportController.h"
#include "ui/controllers/UiDomainController.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

UiImportController::UiImportController(std::shared_ptr<ImportController> coreController, QObject* parent)
    : QObject(parent), coreController_(std::move(coreController))
    , runs_(this)
{
    connect(&importWatcher_, &QFutureWatcher<std::pair<std::shared_ptr<Statement>, std::string>>::finished,
            this, &UiImportController::onImportFutureFinished);
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
    emit stateChanged();
}

void UiImportController::clearDraft()
{
    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }
    emit stateChanged();
}

void UiImportController::cancelImport()
{
    if (!isRunning_) return;
    canceled_ = true;
    phase_ = QStringLiteral("Cancel requested");
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
    if (!coreController_) {
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

    // create a progress callback that posts updates to the UI thread
    auto progressCb = [this](double p, const std::string& phaseMsg) {
        // clamp p
        if (p < 0.0) p = 0.0; if (p > 1.0) p = 1.0;
        QString phase = QString::fromStdString(phaseMsg);
        QMetaObject::invokeMethod(this, "updateProgress", Qt::QueuedConnection, Q_ARG(double, p), Q_ARG(QString, phase));
    };

    // Run import on a worker thread and capture result or error message
    importFuture_ = QtConcurrent::run([this, p, runRoot, runIdPrefix, progressCb]() -> std::pair<std::shared_ptr<Statement>, std::string> {
        try {
            // call core ImportController with progress callback
            auto imported = coreController_->import(ImportController::ImportType::Statement, p, runRoot, runIdPrefix,
                                                   std::function<void(double, const std::string&)>(progressCb));
            return { imported, std::string() };
        } catch (const std::exception& e) {
            return { nullptr, std::string(e.what()) };
        } catch (...) {
            return { nullptr, std::string("Unknown import error") };
        }
    });

    importWatcher_.setFuture(importFuture_);
}

void UiImportController::updateProgress(double p, const QString& phase)
{
    // ignore updates when cancelled or not running
    if (!isRunning_ || canceled_) return;
    progress_ = p;
    if (!phase.isEmpty()) phase_ = phase;
    emit stateChanged();
}

void UiImportController::onImportFutureFinished()
{
    auto pairRes = importFuture_.result();
    const auto& imported = pairRes.first;
    const std::string err = pairRes.second;

    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (canceled_) {
        phase_ = QStringLiteral("Import canceled");
        error_ = QStringLiteral("");
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Canceled"), QStringLiteral(""));
        emit stateChanged();
        emit importFailed(QStringLiteral("Canceled"));
        return;
    }

    if (!imported) {
        error_ = QString::fromUtf8(err.empty() ? std::string("Import failed") : err);
        phase_ = QStringLiteral("Import failed");
        isRunning_ = false;
        progress_ = 0.0;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Failed"), error_);
        emit stateChanged();
        emit importFailed(error_);
        return;
    }

    // Build the UI draft from imported data
    draft_ = new StatementDraft(this);
    draft_->setName(QFileInfo(selectedFile_).baseName());

    std::vector<TransactionDraft> txs;
    txs.reserve(imported->transactions.size());
    for (const auto& tx : imported->transactions) {
        TransactionDraft d;
        d.name = QString::fromStdString(tx.name);
        d.bookingDate = QString::fromStdString(tx.bookingDate);
        d.valuta = QString::fromStdString(tx.valuta);
        d.amount = tx.amount;
        d.description = QString::fromStdString(tx.description);
        d.actorId = QString();
        d.actorProposal = QString::fromStdString(tx.actorProposal);

        d.metadata = QString::fromStdString(tx.metadata);
        d.proofImagePath = QString::fromStdString(tx.proofImagePath);

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
