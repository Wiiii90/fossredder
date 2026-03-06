#include "ui/import/ImportJobBridge.h"

#include "core/jobs/JobSystem.h"

namespace ui::importing {

ImportJobBridge::ImportJobBridge(std::shared_ptr<core::jobs::JobSystem> jobSystem)
    : jobSystem_(std::move(jobSystem))
{
}

ImportJobBridge::~ImportJobBridge()
{
    clearSubscription(ExceptionReporter{});
}

bool ImportJobBridge::startStatementImport(const core::jobs::ImportStatementJobSpec& spec,
                                           core::jobs::JobEventCallback callback,
                                           const ExceptionReporter& exceptionReporter)
{
    if (!jobSystem_) return false;

    try {
        clearSubscription(exceptionReporter);

        const auto jobId = jobSystem_->startImportStatement(spec);
        currentJobId_ = QString::fromStdString(jobId);

        currentSubId_ = jobSystem_->manager().subscribe(jobId, [this, callback = std::move(callback)](const core::jobs::JobEvent& event) {
            if (currentJobId_.isEmpty()) return;
            if (QString::fromStdString(event.jobId) != currentJobId_) return;
            if (callback) callback(event);
        });

        return true;
    } catch (const std::exception&) {
        currentSubId_ = 0;
        currentJobId_.clear();
        if (exceptionReporter) exceptionReporter("ui::importing::ImportJobBridge::startStatementImport", std::current_exception());
        return false;
    } catch (...) {
        currentSubId_ = 0;
        currentJobId_.clear();
        if (exceptionReporter) exceptionReporter("ui::importing::ImportJobBridge::startStatementImport", std::current_exception());
        return false;
    }
}

void ImportJobBridge::cancelCurrent()
{
    if (!jobSystem_ || currentJobId_.isEmpty()) return;
    jobSystem_->manager().cancel(currentJobId_.toStdString());
}

void ImportJobBridge::clearSubscription(const ExceptionReporter& exceptionReporter)
{
    if (!jobSystem_ || currentSubId_ == 0 || currentJobId_.isEmpty()) {
        currentSubId_ = 0;
        currentJobId_.clear();
        return;
    }

    try {
        jobSystem_->manager().unsubscribe(currentJobId_.toStdString(), currentSubId_);
    } catch (const std::exception&) {
        if (exceptionReporter) exceptionReporter("ui::importing::ImportJobBridge::clearSubscription", std::current_exception());
    } catch (...) {
        if (exceptionReporter) exceptionReporter("ui::importing::ImportJobBridge::clearSubscription", std::current_exception());
    }

    currentSubId_ = 0;
    currentJobId_.clear();
}

std::shared_ptr<Statement> ImportJobBridge::statementResult() const
{
    if (!jobSystem_ || currentJobId_.isEmpty()) return nullptr;
    return jobSystem_->manager().statementResult(currentJobId_.toStdString());
}

std::map<std::string, std::vector<uint8_t>> ImportJobBridge::takeArtifacts()
{
    if (!jobSystem_ || currentJobId_.isEmpty()) return {};
    return jobSystem_->manager().takeStatementArtifacts(currentJobId_.toStdString());
}

}