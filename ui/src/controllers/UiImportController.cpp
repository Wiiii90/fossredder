#include "ui/controllers/UiImportController.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>

#include "core/controllers/ImportController.h"
#include "ui/controllers/UiDomainController.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

UiImportController::UiImportController(std::shared_ptr<ImportController> coreController, QObject* parent)
    : QObject(parent), coreController_(std::move(coreController))
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
    emit stateChanged();
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

    error_.clear();
    phase_ = QStringLiteral("Starting import...");
    progress_ = 0.1;
    isRunning_ = true;
    emit stateChanged();

    const QByteArray nativePath = QFile::encodeName(selectedFile_);
    std::string p(nativePath.constData(), static_cast<size_t>(nativePath.size()));

    const auto now = QDateTime::currentDateTime().toString(Qt::ISODate);

    try {
        auto imported = coreController_->import(ImportController::ImportType::Statement, p);

        if (domain_ && imported) {
            const QString statementName = QFileInfo(selectedFile_).baseName();
            const QString sid = domain_->addStatement(statementName, QString(), QString());
            if (!sid.isEmpty()) {
                lastResultStatementId_ = sid;

                for (const auto& tx : imported->transactions) {
                    domain_->addTransactionWithStatus(QString::fromStdString(tx.name),
                                                      QString::fromStdString(tx.bookingDate),
                                                      tx.amount,
                                                      QString::fromStdString(tx.description.empty() ? tx.name : tx.description),
                                                      sid,
                                                      static_cast<int>(Transaction::Status::Unverified));
                }
            }
        }

        phase_ = QStringLiteral("Import finished");
        progress_ = 1.0;
        isRunning_ = false;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Success"), QStringLiteral(""));
        emit stateChanged();
        emit importFinished();
    } catch (const std::exception& e) {
        error_ = QString::fromUtf8(e.what());
        phase_ = QStringLiteral("Import failed");
        isRunning_ = false;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Failed"), error_);
        emit stateChanged();
        emit importFailed(error_);
    } catch (...) {
        error_ = QStringLiteral("Import failed");
        phase_ = QStringLiteral("Import failed");
        isRunning_ = false;
        runs_.addRun(now, QStringLiteral("Statement"), selectedFile_, QStringLiteral("Failed"), error_);
        emit stateChanged();
        emit importFailed(error_);
    }
}
