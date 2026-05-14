/**
 * @file ui/include/ui/import/ImportState.h
 * @brief Declares the mutable UI-side state for the statement import workflow.
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

#include <QByteArray>
#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/import/draft/IImportMatcherService.h"
#include "ui/models/StatementDraft.h"

namespace core::domain { class Statement; }
class QObject;

namespace ui::importing {

class ImportState {
public:
    ImportState() = default;

    bool isRunning() const noexcept { return isRunning_; }
    double progress() const noexcept { return progress_; }
    const QString& phase() const noexcept { return phase_; }
    const QString& error() const noexcept { return error_; }
    int currentPage() const noexcept { return currentPage_; }
    int pageCount() const noexcept { return pageCount_; }
    const QString& selectedFile() const noexcept { return selectedFile_; }
    const QStringList& queuedFiles() const noexcept { return queuedFiles_; }
    StatementDraft* draft() const noexcept { return draft_; }
    int artifactCount() const noexcept { return artifactCount_; }
    bool cancelRequested() const noexcept { return canceled_; }

    bool setSelectedFile(const QString& path);
    bool addFiles(const QStringList& paths);
    bool resetStatus();
    bool clearDraft();

    QString currentRunFile() const;
    QString takeSelectedFileForStart();
    bool takeNextQueuedFile(QString& nextFile);

    void beginImport(const QString& path);
    void rejectStart(const QString& errorMessage);
    void beginCancel(bool clearQueue);
    void recordCanceled(const QString& now);
    void recordFailed(const QString& now, const QString& errorMessage);
    void recordFinished(const QString& now);
    bool populateDraft(const QString& now,
                       const std::shared_ptr<core::domain::Statement>& statement,
                        const core::domain::catalog::WorkspaceCatalog& state,
                       const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                       const std::map<std::string, std::vector<uint8_t>>& artifacts,
                       const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                       const QString& draftId,
                       int currentTransactionIndex,
                       QObject* parent);
    bool restoreDraft(const std::shared_ptr<core::domain::Statement>& statement,
                      const core::domain::catalog::WorkspaceCatalog& state,
                      const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                      const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                      const QString& draftId,
                      int currentTransactionIndex,
                      QObject* parent);
    void setMatcherService(std::shared_ptr<core::application::importing::draft::IImportMatcherService> matcherService);
    void updateProgress(double progress, const QString& phase, const QRegularExpression& pagePattern);

private:
    void clearDraftObject();
    void clearTransientImportState();
    void resetCancellationState();
    void storeArtifacts(const std::map<std::string, std::vector<uint8_t>>& artifacts);
    void finalizeRun(const QString& phase);

    bool isRunning_ = false;
    double progress_ = 0.0;
    QString phase_;
    QString error_;
    int currentPage_ = 0;
    int pageCount_ = 0;
    QString selectedFile_;
    QStringList queuedFiles_;
    StatementDraft* draft_ = nullptr;
    int artifactCount_ = 0;
    bool canceled_ = false;
    bool cancelClearsQueue_ = false;
    QString currentImportFile_;
    std::shared_ptr<core::application::importing::draft::IImportMatcherService> matcherService_;
};

}
