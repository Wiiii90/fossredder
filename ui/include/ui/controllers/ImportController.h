#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <memory>

#include "core/errors/IErrorReporter.h"
#include "ui/import/ImportJobBridge.h"
#include "ui/import/ImportState.h"
#include "ui/models/ImportRunList.h"

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

    bool isRunning() const noexcept { return state_.isRunning(); }
    double progress() const noexcept { return state_.progress(); }
    QString phase() const { return state_.phase(); }
    QString error() const { return state_.error(); }
    int currentPage() const noexcept { return state_.currentPage(); }
    int pageCount() const noexcept { return state_.pageCount(); }
    QString selectedFile() const { return state_.selectedFile(); }
    void setSelectedFile(const QString& path);
    int queuedCount() const noexcept { return state_.queuedFiles().size(); }
    QStringList queuedFiles() const { return state_.queuedFiles(); }
    StatementDraft* draft() const noexcept { return state_.draft(); }

    Q_INVOKABLE void startStatementImport();
    Q_INVOKABLE void addFiles(const QStringList& paths);
    Q_INVOKABLE void resetStatus();
    Q_INVOKABLE void clearDraft();
    Q_INVOKABLE void cancelImport();
    Q_INVOKABLE void cancelAllImports();
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);

    ImportRunList* runs() noexcept { return &runs_; }
    QByteArray artifactBytes(const QString& key) const { return state_.artifactBytes(key); }

signals:
    void stateChanged();
    void importFinished();
    void importCanceled();
    void importFailed(const QString& error);

private slots:
    void updateProgress(double p, const QString& phase);
    void onJobTerminal(core::jobs::JobState state, const QString& message);

private:
    void rejectImportStart(const QString& errorMessage, const char* traceMessage);
    void requestImportCancellation(bool clearQueue, const char* origin, const char* traceMessage);
    void handleJobEvent(const core::jobs::JobEvent& event);
    void handleImportCanceled(const QString& now);
    void handleImportFailed(const QString& now, const QString& errorMessage, const char* traceMessage);
    bool populateDraftFromResult(const QString& now);
    ImportRunList runs_;
    importing::ImportState state_;
    importing::ImportJobBridge jobBridge_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;

    void startNextQueuedImport();
    void startImportForFile(const QString& path);
    void reportException(const char* origin, std::exception_ptr exception) const;
};

}
