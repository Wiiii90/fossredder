#include "ui/import/ImportState.h"

#include <QCoreApplication>

#include "ui/config/Defaults.h"
#include "ui/controllers/ControllerContracts.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/text/Text.h"

namespace ui::importing {

ImportState::ImportState(ImportRunList& runs)
    : runs_(runs)
{
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
    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }

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

void ImportState::beginImport(const QString& path)
{
    selectedFile_ = path;
    currentImportFile_ = path;

    if (draft_) {
        draft_->deleteLater();
        draft_ = nullptr;
    }

    artifacts_.clear();
    error_.clear();
    phase_ = QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importPhases::kStarting);
    progress_ = ui::config::importProgress::kInitial;
    currentPage_ = ui::config::importPaging::kNone;
    pageCount_ = ui::config::importPaging::kNone;
    isRunning_ = true;
    canceled_ = false;
    cancelClearsQueue_ = false;
}

void ImportState::rejectStart(const QString& errorMessage)
{
    error_ = errorMessage;
    queuedFiles_.clear();
    currentImportFile_.clear();
    phase_.clear();
    clearTransientImportState();
    canceled_ = false;
    cancelClearsQueue_ = false;
}

void ImportState::beginCancel(bool clearQueue)
{
    if (!isRunning_) return;
    canceled_ = true;
    cancelClearsQueue_ = clearQueue;
    if (clearQueue) queuedFiles_.clear();
    phase_ = QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importPhases::kStopping);
}

void ImportState::appendRun(const QString& now, const QString& status, const QString& message)
{
    runs_.addRun(now,
                 QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importRuns::kTypeStatement),
                 currentRunFile(),
                 status,
                 message);
}

void ImportState::recordCanceled(const QString& now)
{
    phase_ = QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importPhases::kCanceled);
    error_.clear();
    clearTransientImportState();
    if (cancelClearsQueue_) queuedFiles_.clear();
    appendRun(now, QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importRuns::kStatusCanceled), {});
    selectedFile_.clear();
    currentImportFile_.clear();
    canceled_ = false;
    cancelClearsQueue_ = false;
}

void ImportState::recordFailed(const QString& now, const QString& errorMessage)
{
    error_ = errorMessage;
    phase_ = QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importPhases::kFailed);
    clearTransientImportState();
    queuedFiles_.clear();
    appendRun(now, QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importRuns::kStatusFailed), error_);
    currentImportFile_.clear();
    canceled_ = false;
    cancelClearsQueue_ = false;
}

bool ImportState::populateDraft(const QString& now,
                                const std::shared_ptr<Statement>& statement,
                                const std::map<std::string, std::vector<uint8_t>>& artifacts,
                                QObject* parent)
{
    if (!statement) return false;

    artifacts_.clear();
    for (const auto& [key, value] : artifacts) {
        artifacts_.insert(QString::fromStdString(key),
                          value.empty()
                              ? QByteArray()
                              : QByteArray(reinterpret_cast<const char*>(value.data()), static_cast<int>(value.size())));
    }

    draft_ = createStatementDraft(currentImportFile_, statement, parent);
    phase_ = QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importPhases::kFinished);
    progress_ = ui::config::importProgress::kMaximum;
    isRunning_ = false;
    appendRun(now, QCoreApplication::translate(ui::text::contexts::kImportState, ui::text::importRuns::kStatusSuccess), {});
    currentImportFile_.clear();
    canceled_ = false;
    cancelClearsQueue_ = false;
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
