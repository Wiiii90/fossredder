/**
 * @file ui/src/workflows/import/ImportWorkflowState.cpp
 * @brief Implements the mutable UI-side state for the statement import workflow.
 */

#include "ui/workflows/import/ImportWorkflowState.h"

#include "ui/shared/config/Defaults.h"
#include "ui/adapters/core/ImportDraftMapper.h"
#include "ui/shared/text/Text.h"

namespace ui::importing {

void ImportWorkflowState::clearDraftObject()
{
    if (!draft_) return;
    draft_->deleteLater();
    draft_ = nullptr;
}

void ImportWorkflowState::setMatcherService(std::shared_ptr<core::application::importing::draft::IImportMatcherService> matcherService)
{
    matcherService_ = std::move(matcherService);
}

bool ImportWorkflowState::setSelectedFile(const QString& path)
{
    if (selectedFile_ == path) return false;
    selectedFile_ = path;
    return true;
}

bool ImportWorkflowState::addFiles(const QStringList& paths)
{
    QStringList cleaned;
    cleaned.reserve(paths.size());
    for (const auto& path : paths) {
        const auto trimmed = path.trimmed();
        if (trimmed.isEmpty()) continue;
        cleaned.push_back(trimmed);
    }
    if (cleaned.isEmpty()) return false;

    bool changed = false;
    const bool hasSelection = !selectedFile_.trimmed().isEmpty();
    int startIndex = 0;
    if (!hasSelection) {
        changed = setSelectedFile(cleaned.front()) || changed;
        startIndex = 1;
    }

    for (int i = startIndex; i < cleaned.size(); ++i) {
        queuedFiles_.push_back(cleaned[i]);
        changed = true;
    }

    return changed;
}

bool ImportWorkflowState::resetStatus()
{
    if (isRunning_) return false;

    phase_.clear();
    error_.clear();
    progress_ = ui::config::importProgress::kMinimum;
    canceled_ = false;
    cancelClearsQueue_ = false;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
    selectedFile_.clear();
    currentImportFile_.clear();
    queuedFiles_.clear();
    artifactCount_ = 0;
    return true;
}

bool ImportWorkflowState::clearDraft()
{
    clearDraftObject();

    artifactCount_ = 0;
    if (!isRunning_ && queuedFiles_.isEmpty()) {
        selectedFile_.clear();
        currentImportFile_.clear();
    }
    return !isRunning_ && !queuedFiles_.isEmpty();
}

void ImportWorkflowState::recordFinished(const QString& now)
{
    (void)now;
    error_.clear();
    finalizeRun(ui::text::importPhases::finished());
    selectedFile_.clear();
    currentImportFile_.clear();
}

QString ImportWorkflowState::currentRunFile() const
{
    return currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_;
}

QString ImportWorkflowState::takeSelectedFileForStart()
{
    const QString trimmed = selectedFile_.trimmed();
    if (trimmed.isEmpty()) return {};
    selectedFile_.clear();
    return trimmed;
}

bool ImportWorkflowState::takeNextQueuedFile(QString& nextFile)
{
    if (isRunning_ || queuedFiles_.isEmpty()) return false;
    nextFile = queuedFiles_.takeFirst();
    return true;
}

void ImportWorkflowState::clearTransientImportState()
{
    isRunning_ = false;
    progress_ = ui::config::importProgress::kMinimum;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
}

void ImportWorkflowState::resetCancellationState()
{
    canceled_ = false;
    cancelClearsQueue_ = false;
}

void ImportWorkflowState::storeArtifacts(const std::map<std::string, std::vector<uint8_t>>& artifacts)
{
    artifactCount_ = static_cast<int>(artifacts.size());
}

void ImportWorkflowState::finalizeRun(const QString& phase)
{
    phase_ = phase;
    clearTransientImportState();
    resetCancellationState();
}

void ImportWorkflowState::beginImport(const QString& path)
{
    selectedFile_ = path;
    currentImportFile_ = path;

    artifactCount_ = 0;
    error_.clear();
    phase_ = ui::text::importPhases::starting();
    progress_ = ui::config::importProgress::kInitial;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
    isRunning_ = true;
    resetCancellationState();
}

void ImportWorkflowState::rejectStart(const QString& errorMessage)
{
    error_ = errorMessage;
    queuedFiles_.clear();
    currentImportFile_.clear();
    phase_.clear();
    clearTransientImportState();
    resetCancellationState();
}

void ImportWorkflowState::beginCancel(bool clearQueue)
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = clearQueue;
    if (clearQueue) queuedFiles_.clear();
    phase_ = ui::text::importPhases::stopping();
}

void ImportWorkflowState::recordCanceled(const QString& now)
{
    (void)now;
    error_.clear();
    if (cancelClearsQueue_) queuedFiles_.clear();
    finalizeRun(ui::text::importPhases::canceled());
    selectedFile_.clear();
    currentImportFile_.clear();
}

void ImportWorkflowState::recordFailed(const QString& now, const QString& errorMessage)
{
    (void)now;
    error_ = errorMessage;
    queuedFiles_.clear();
    finalizeRun(ui::text::importPhases::failed());
    currentImportFile_.clear();
}

bool ImportWorkflowState::populateDraft(const QString& now,
                                const std::shared_ptr<core::domain::Statement>& statement,
                                const core::domain::catalog::WorkspaceCatalog& state,
                                const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                                const std::map<std::string, std::vector<uint8_t>>& artifacts,
                                const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                                const QString& draftId,
                                int currentTransactionIndex,
                                QObject* parent)
{
    (void)now;
    if (!statement) return false;

    storeArtifacts(artifacts);

    clearDraftObject();
    draft_ = createStatementDraft(currentImportFile_,
                                  statement,
                                  state,
                                  transactions,
                                  matcherService ? matcherService : matcherService_,
                                  draftId,
                                  currentTransactionIndex,
                                  parent);
    recordFinished(now);
    return true;
}

bool ImportWorkflowState::restoreDraft(const std::shared_ptr<core::domain::Statement>& statement,
                               const core::domain::catalog::WorkspaceCatalog& state,
                               const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                               const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                               const QString& draftId,
                               int currentTransactionIndex,
                               QObject* parent)
{
    if (!statement) return false;

    clearDraftObject();
    draft_ = createStatementDraft(currentRunFile(),
                                  statement,
                                  state,
                                  transactions,
                                  matcherService ? matcherService : matcherService_,
                                  draftId,
                                  currentTransactionIndex,
                                  parent);
    return draft_ != nullptr;
}

void ImportWorkflowState::updateProgress(double progress, const QString& phase, const QRegularExpression& pagePattern)
{
    if (!isRunning_ || canceled_) return;

    progress_ = progress;
    if (phase.isEmpty()) return;

    phase_ = phase;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;

    const auto match = pagePattern.match(phase);
    if (!match.hasMatch()) return;

    bool pageOk = false;
    bool countOk = false;
    const int currentPage = match.captured(1).toInt(&pageOk);
    const int totalPages = match.captured(2).toInt(&countOk);
    if (!pageOk || !countOk) return;

    currentPage_ = currentPage;
    pageCount_ = totalPages;
}

}
