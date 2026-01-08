#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <QPointer>
#include <QFuture>
#include <QFutureWatcher>
#include <atomic>

#include "ui/models/ImportRunListModel.h"
#include "ui/models/StatementDraft.h"

class ImportController;
class UiDomainController;
class Statement; // forward declare core Statement

class UiImportController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
    Q_PROPERTY(QString phase READ phase NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)

    Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY stateChanged)
    Q_PROPERTY(QStringList profiles READ profiles CONSTANT)
    Q_PROPERTY(QString selectedProfile READ selectedProfile WRITE setSelectedProfile NOTIFY stateChanged)

    Q_PROPERTY(ImportRunListModel* runs READ runs CONSTANT)

    Q_PROPERTY(StatementDraft* draft READ draft NOTIFY stateChanged)

public:
    explicit UiImportController(std::shared_ptr<ImportController> coreController, QObject* parent = nullptr);

    void setDomainController(UiDomainController* domain);

    bool isRunning() const noexcept { return isRunning_; }
    double progress() const noexcept { return progress_; }
    QString phase() const { return phase_; }
    QString error() const { return error_; }

    QString selectedFile() const { return selectedFile_; }
    void setSelectedFile(const QString& path);

    QStringList profiles() const;
    QString selectedProfile() const { return selectedProfile_; }
    void setSelectedProfile(const QString& profile);

    StatementDraft* draft() const noexcept { return draft_; }

    Q_INVOKABLE void startStatementImport();
    Q_INVOKABLE void resetStatus();
    Q_INVOKABLE void clearDraft();
    Q_INVOKABLE void cancelImport();

    ImportRunListModel* runs() noexcept { return &runs_; }

signals:
    void stateChanged();
    void importFinished();
    void importFailed(const QString& error);

private slots:
    void onImportFutureFinished();
    void updateProgress(double p, const QString& phase);

private:
    std::shared_ptr<ImportController> coreController_;
    QPointer<UiDomainController> domain_;

    bool isRunning_ = false;
    double progress_ = 0.0;
    QString phase_;
    QString error_;

    QString selectedFile_;
    QString selectedProfile_ = QStringLiteral("Default");

    ImportRunListModel runs_;

    StatementDraft* draft_ = nullptr;

    // Async handling: return pair(result, errorMessage)
    QFuture<std::pair<std::shared_ptr<Statement>, std::string>> importFuture_;
    QFutureWatcher<std::pair<std::shared_ptr<Statement>, std::string>> importWatcher_;

    bool canceled_ = false;
    std::shared_ptr<std::atomic<bool>> cancelFlag_ = nullptr;
};
