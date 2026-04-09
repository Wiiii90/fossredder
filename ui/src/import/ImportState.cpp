/**
 * @file ui/src/import/ImportState.cpp
 * @brief Implements the mutable UI-side state for the statement import workflow.
 */

#include "ui/import/ImportState.h"

#include "ui/config/Defaults.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/text/Text.h"

namespace ui::importing {

ImportState::ImportState(ImportRunList& runs) : runs_(runs) {}

void ImportState::clearDraftObject()
{
    if (!draft_) return;
    draft_->deleteLater();
    draft_ = nullptr;
}

bool ImportState::setSelectedFile(const QString& path)
{
    if (selectedFile_ == path) return false;
    selectedFile_ = path;
    return true;
}

bool ImportState::addFiles(const QStringList& paths)
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

bool ImportState::resetStatus()
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
    artifacts_.clear();
    return true;
}

bool ImportState::clearDraft()
{
    clearDraftObject();

    artifacts_.clear();
    if (!isRunning_ && queuedFiles_.isEmpty()) {
        selectedFile_.clear();
        currentImportFile_.clear();
    }

    return !isRunning_ && !queuedFiles_.isEmpty();
}

QByteArray ImportState::artifactBytes(const QString& key) const
{
    const auto it = artifacts_.find(key);
    return it == artifacts_.end() ? QByteArray() : it.value();
}

QString ImportState::currentRunFile() const
{
    return currentImportFile_.isEmpty() ? selectedFile_ : currentImportFile_;
}

QString ImportState::takeSelectedFileForStart()
{
    const QString trimmed = selectedFile_.trimmed();
    if (trimmed.isEmpty()) return {};
    selectedFile_.clear();
    return trimmed;
}

bool ImportState::takeNextQueuedFile(QString& nextFile)
{
    if (isRunning_ || draft_ || queuedFiles_.isEmpty()) return false;
    nextFile = queuedFiles_.takeFirst();
    return true;
}

void ImportState::clearTransientImportState()
{
    isRunning_ = false;
    progress_ = ui::config::importProgress::kMinimum;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
}

void ImportState::resetCancellationState()
{
    canceled_ = false;
    cancelClearsQueue_ = false;
}

void ImportState::storeArtifacts(const std::map<std::string, std::vector<uint8_t>>& artifacts)
{
    artifacts_.clear();
    for (const auto& [key, value] : artifacts) {
        artifacts_.insert(QString::fromStdString(key),
                          value.empty()
                              ? QByteArray()
                              : QByteArray(reinterpret_cast<const char*>(value.data()), static_cast<int>(value.size())));
    }
}

void ImportState::finalizeRun(const QString& now, const QString& phase, const QString& status, const QString& message)
{
    phase_ = phase;
    clearTransientImportState();
    appendRun(now, status, message);
    resetCancellationState();
}

void ImportState::beginImport(const QString& path)
{
    selectedFile_ = path;
    currentImportFile_ = path;

    clearDraftObject();

    artifacts_.clear();
    error_.clear();
    phase_ = ui::text::importPhases::starting();
    progress_ = ui::config::importProgress::kInitial;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
    isRunning_ = true;
    resetCancellationState();
}

void ImportState::rejectStart(const QString& errorMessage)
{
    error_ = errorMessage;
    queuedFiles_.clear();
    currentImportFile_.clear();
    phase_.clear();
    clearTransientImportState();
    resetCancellationState();
}

void ImportState::beginCancel(bool clearQueue)
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = clearQueue;
    if (clearQueue) queuedFiles_.clear();
    phase_ = ui::text::importPhases::stopping();
}

void ImportState::appendRun(const QString& now, const QString& status, const QString& message)
{
    runs_.addRun(now,
                 ui::text::importRuns::typeStatement(),
                 currentRunFile(),
                 status,
                 message);
}

void ImportState::recordCanceled(const QString& now)
{
    error_.clear();
    if (cancelClearsQueue_) queuedFiles_.clear();
    finalizeRun(now,
                ui::text::importPhases::canceled(),
                ui::text::importRuns::statusCanceled());
    selectedFile_.clear();
    currentImportFile_.clear();
}

void ImportState::recordFailed(const QString& now, const QString& errorMessage)
{
    error_ = errorMessage;
    queuedFiles_.clear();
    finalizeRun(now,
                ui::text::importPhases::failed(),
                ui::text::importRuns::statusFailed(),
                error_);
    currentImportFile_.clear();
}

bool ImportState::populateDraft(const QString& now,
                                const std::shared_ptr<core::domain::Statement>& statement,
                                const core::domain::AppState& state,
                                const std::vector<core::domain::TransactionDraft>& transactions,
                                const std::map<std::string, std::vector<uint8_t>>& artifacts,
                                QObject* parent)
{
    if (!statement) return false;

    storeArtifacts(artifacts);

    draft_ = createStatementDraft(currentImportFile_, statement, state, transactions, parent);
    progress_ = ui::config::importProgress::kMaximum;
    finalizeRun(now,
                ui::text::importPhases::finished(),
                ui::text::importRuns::statusSuccess());
    currentImportFile_.clear();
    return true;
}

void ImportState::updateProgress(double progress, const QString& phase, const QRegularExpression& pagePattern)
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
