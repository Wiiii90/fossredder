/**
 * @file ui/src/import/ImportJobBridge.cpp
 * @brief Implements the bridge between the UI import workflow and the core job system.
 */

#include "ui/import/ImportJobBridge.h"

#include "core/jobs/JobSystem.h"
#include "ui/observability/Origins.h"
#include "ui/support/StringConversions.h"

namespace ui::importing {

namespace {

void reportBridgeException(const ExceptionReporter &exceptionReporter,
                           const char *origin) {
  if (exceptionReporter)
    exceptionReporter(origin, std::current_exception());
}

} // namespace

ImportJobBridge::ImportJobBridge(
    std::shared_ptr<core::jobs::JobSystem> jobSystem)
    : jobSystem_(std::move(jobSystem)) {}

ImportJobBridge::~ImportJobBridge() { clearSubscription(); }

void ImportJobBridge::setExceptionReporter(ExceptionReporter reporter) {
  exceptionReporter_ = std::move(reporter);
}

bool ImportJobBridge::startStatementImport(
    const core::jobs::ImportStatementJobSpec &spec,
    core::jobs::JobEventCallback callback) {
  if (!jobSystem_)
    return false;

  try {
    clearSubscription();

    const auto jobId = jobSystem_->startImportStatement(spec);
    currentJobId_ = QString::fromStdString(jobId);

    currentSubId_ = jobSystem_->subscribe(
        jobId, [this, callback = std::move(callback)](
                   const core::jobs::JobEvent &event) {
          if (currentJobId_.isEmpty())
            return;
          if (QString::fromStdString(event.jobId) != currentJobId_)
            return;
          if (callback)
            callback(event);
        });

    return true;
  } catch (const std::exception &) {
    currentSubId_ = 0;
    currentJobId_.clear();
    reportBridgeException(exceptionReporter_,
                          observability::origins::service::importJobBridge::kStartImport);
    return false;
  } catch (...) {
    currentSubId_ = 0;
    currentJobId_.clear();
    reportBridgeException(exceptionReporter_,
                          observability::origins::service::importJobBridge::kStartImport);
    return false;
  }
}

void ImportJobBridge::cancelCurrent() {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return;
  jobSystem_->cancel(strings::toStdString(currentJobId_));
}

void ImportJobBridge::clearSubscription() {
  if (!jobSystem_ || currentSubId_ == 0 || currentJobId_.isEmpty()) {
    currentSubId_ = 0;
    currentJobId_.clear();
    return;
  }

  try {
    jobSystem_->unsubscribe(strings::toStdString(currentJobId_), currentSubId_);
  } catch (const std::exception &) {
    reportBridgeException(exceptionReporter_,
                          observability::origins::service::importJobBridge::kClearSubscription);
  } catch (...) {
    reportBridgeException(exceptionReporter_,
                          observability::origins::service::importJobBridge::kClearSubscription);
  }

  currentSubId_ = 0;
  currentJobId_.clear();
}

std::shared_ptr<core::domain::Statement> ImportJobBridge::statementResult() const {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return nullptr;
  return jobSystem_->statementResult(strings::toStdString(currentJobId_));
}

std::vector<ImportedTransaction> ImportJobBridge::statementTransactions() const {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return {};
  return jobSystem_->statementTransactions(strings::toStdString(currentJobId_));
}

std::map<std::string, std::vector<uint8_t>> ImportJobBridge::takeArtifacts() {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return {};
  return jobSystem_->takeStatementArtifacts(strings::toStdString(currentJobId_));
}

} // namespace ui::importing