#pragma once

#include <map>
#include <memory>
#include <vector>

#include <QByteArray>
#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "core/import/ImportedTransaction.h"
#include "ui/models/ImportRunList.h"
#include "ui/models/StatementDraft.h"

namespace core::domain { class Statement; }
class QObject;

namespace ui::importing {

class ImportState {
public:
    explicit ImportState(ImportRunList& runs);

    bool isRunning() const noexcept { return isRunning_; }
    double progress() const noexcept { return progress_; }
    const QString& phase() const noexcept { return phase_; }
    const QString& error() const noexcept { return error_; }
    int currentPage() const noexcept { return currentPage_; }
    int pageCount() const noexcept { return pageCount_; }
    const QString& selectedFile() const noexcept { return selectedFile_; }
    const QStringList& queuedFiles() const noexcept { return queuedFiles_; }
    StatementDraft* draft() const noexcept { return draft_; }
    int artifactCount() const noexcept { return artifacts_.size(); }
    bool cancelRequested() const noexcept { return canceled_; }

    bool setSelectedFile(const QString& path);
    bool addFiles(const QStringList& paths);
    bool resetStatus();
    bool clearDraft();

    QByteArray artifactBytes(const QString& key) const;
    QString currentRunFile() const;
    QString takeSelectedFileForStart();
    bool takeNextQueuedFile(QString& nextFile);

    void beginImport(const QString& path);
    void rejectStart(const QString& errorMessage);
    void beginCancel(bool clearQueue);
    void recordCanceled(const QString& now);
    void recordFailed(const QString& now, const QString& errorMessage);
    bool populateDraft(const QString& now,
                       const std::shared_ptr<core::domain::Statement>& statement,
                       const std::vector<ImportedTransaction>& transactions,
                       const std::map<std::string, std::vector<uint8_t>>& artifacts,
                       QObject* parent);
    void updateProgress(double progress, const QString& phase, const QRegularExpression& pagePattern);

private:
    void clearTransientImportState();
    void appendRun(const QString& now, const QString& status, const QString& message);

    ImportRunList& runs_;
    bool isRunning_ = false;
    double progress_ = 0.0;
    QString phase_;
    QString error_;
    int currentPage_ = 0;
    int pageCount_ = 0;
    QString selectedFile_;
    QStringList queuedFiles_;
    StatementDraft* draft_ = nullptr;
    QHash<QString, QByteArray> artifacts_;
    bool canceled_ = false;
    bool cancelClearsQueue_ = false;
    QString currentImportFile_;
};

}
