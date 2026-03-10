#include "ui/import/ImportJobBridge.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/jobs/JobSystem.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"

namespace ui::importing {

namespace {

void reportBridgeException(const ExceptionReporter &exceptionReporter,
                           const char *origin) {
  if (exceptionReporter) {
    exceptionReporter(origin, std::current_exception());
    return;
  }

  core::errors::reportException(core::errors::ErrorSeverity::Error,
                                core::errors::codes::ExceptionError, origin,
                                std::current_exception());
}

} // namespace

ImportJobBridge::ImportJobBridge(
    std::shared_ptr<core::jobs::JobSystem> jobSystem)
    : jobSystem_(std::move(jobSystem)) {}

ImportJobBridge::~ImportJobBridge() { clearSubscription(ExceptionReporter{}); }

bool ImportJobBridge::startStatementImport(
    const core::jobs::ImportStatementJobSpec &spec,
    core::jobs::JobEventCallback callback,
    const ExceptionReporter &exceptionReporter) {
  if (!jobSystem_)
    return false;

  try {
    clearSubscription(exceptionReporter);

    const auto jobId = jobSystem_->startImportStatement(spec);
    currentJobId_ = QString::fromStdString(jobId);

    currentSubId_ = jobSystem_->manager().subscribe(
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
    reportBridgeException(
        exceptionReporter,
        observability::origins::service::importJobBridge::kStartImport);
    return false;
  } catch (...) {
    currentSubId_ = 0;
    currentJobId_.clear();
    reportBridgeException(
        exceptionReporter,
        observability::origins::service::importJobBridge::kStartImport);
    return false;
  }
}

void ImportJobBridge::cancelCurrent() {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return;
  jobSystem_->manager().cancel(strings::toStdString(currentJobId_));
}

void ImportJobBridge::clearSubscription(
    const ExceptionReporter &exceptionReporter) {
  if (!jobSystem_ || currentSubId_ == 0 || currentJobId_.isEmpty()) {
    currentSubId_ = 0;
    currentJobId_.clear();
    return;
  }

  try {
    jobSystem_->manager().unsubscribe(strings::toStdString(currentJobId_),
                                      currentSubId_);
  } catch (const std::exception &) {
    reportBridgeException(
        exceptionReporter,
        observability::origins::service::importJobBridge::kClearSubscription);
  } catch (...) {
    reportBridgeException(
        exceptionReporter,
        observability::origins::service::importJobBridge::kClearSubscription);
  }

  currentSubId_ = 0;
  currentJobId_.clear();
}

std::shared_ptr<Statement> ImportJobBridge::statementResult() const {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return nullptr;
  return jobSystem_->manager().statementResult(
      strings::toStdString(currentJobId_));
}

std::map<std::string, std::vector<uint8_t>> ImportJobBridge::takeArtifacts() {
  if (!jobSystem_ || currentJobId_.isEmpty())
    return {};
  return jobSystem_->manager().takeStatementArtifacts(
      strings::toStdString(currentJobId_));
}

} // namespace ui::importing