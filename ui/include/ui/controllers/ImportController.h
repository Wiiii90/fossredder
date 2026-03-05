#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <memory>
#include <QHash>
#include <QByteArray>

#include "core/errors/IErrorReporter.h"
#include "ui/import/ImportJobBridge.h"
#include "ui/models/ImportRunList.h"
#include "ui/models/StatementDraft.h"

namespace core { namespace jobs { class JobSystem; } }

namespace ui {

class ImportController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
    Q_PROPERTY(QString phase READ phase NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY stateChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY stateChanged)
    Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY stateChanged)
    Q_PROPERTY(int queuedCount READ queuedCount NOTIFY stateChanged)
    Q_PROPERTY(QStringList queuedFiles READ queuedFiles NOTIFY stateChanged)
    Q_PROPERTY(ImportRunList* runs READ runs CONSTANT)
    Q_PROPERTY(StatementDraft* draft READ draft NOTIFY stateChanged)

public:
    explicit ImportController(std::shared_ptr<core::jobs::JobSystem> jobSystem, QObject* parent = nullptr);

    bool isRunning() const noexcept { return isRunning_; }
    double progress() const noexcept { return progress_; }
    QString phase() const { return phase_; }
    QString error() const { return error_; }
    int currentPage() const noexcept { return currentPage_; }
    int pageCount() const noexcept { return pageCount_; }
    QString selectedFile() const { return selectedFile_; }
    void setSelectedFile(const QString& path);
    int queuedCount() const noexcept { return queuedFiles_.size(); }
    QStringList queuedFiles() const { return queuedFiles_; }
    StatementDraft* draft() const noexcept { return draft_; }

    Q_INVOKABLE void startStatementImport();
    Q_INVOKABLE void addFiles(const QStringList& paths);
    Q_INVOKABLE void resetStatus();
    Q_INVOKABLE void clearDraft();
    Q_INVOKABLE void cancelImport();
    Q_INVOKABLE void cancelAllImports();
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);

    ImportRunList* runs() noexcept { return &runs_; }
    QByteArray artifactBytes(const QString& key) const;

signals:
    void stateChanged();
    void importFinished();
    void importCanceled();
    void importFailed(const QString& error);

private slots:
    void updateProgress(double p, const QString& phase);
    void onJobTerminal(int state, const QString& message);

private:
    std::shared_ptr<core::jobs::JobSystem> jobSystem_;
    bool isRunning_ = false;
    double progress_ = 0.0;
    QString phase_;
    QString error_;
    int currentPage_ = 0;
    int pageCount_ = 0;
    QString selectedFile_;
    QStringList queuedFiles_;
    ImportRunList runs_;
    StatementDraft* draft_ = nullptr;
    QHash<QString, QByteArray> artifacts_;
    bool canceled_ = false;
    bool cancelClearsQueue_ = false;
    importing::ImportJobBridge jobBridge_;
    QString currentImportFile_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;

    void startNextQueuedImport();
    void startImportForFile(const QString& path);
    void reportException(const char* origin, std::exception_ptr exception) const;
};

}
