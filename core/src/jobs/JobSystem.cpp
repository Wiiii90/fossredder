/**
 * @file core/src/jobs/JobSystem.cpp
 * @brief Implements the import job orchestration facade.
 */

#include "core/jobs/JobSystem.h"

#include "core/constants/CoreDefaults.h"
#include "core/import/IImportStatement.h"
#include "JobManager.h"
#include "core/jobs/Scheduler.h"

#include <filesystem>
#include <thread>

namespace {

std::size_t defaultWorkers()
{
    const auto hc = std::thread::hardware_concurrency();
    if (hc == 0) return core::constants::jobs::kFallbackWorkerCount;
    if (hc <= 2) return 1;
    return static_cast<std::size_t>(hc - 1);
}

std::size_t resolveWorkerCount(std::size_t workers)
{
    return workers == 0 ? defaultWorkers() : workers;
}

}

namespace core::jobs {

class JobSystem::Impl {
public:
    Impl(std::shared_ptr<core::importing::IImportStatement> importService, std::size_t workers)
        : importService(std::move(importService))
        , manager()
        , scheduler(resolveWorkerCount(workers), core::constants::jobs::kQueueCapacity)
        , ocrLimiter(std::max<std::size_t>(std::size_t{1}, resolveWorkerCount(workers) / core::constants::jobs::kOcrWorkerDivisor))
    {
    }

    std::shared_ptr<core::importing::IImportStatement> importService;
    JobManager manager;
    Scheduler scheduler;
    SlotLimiter ocrLimiter;
};

JobSystem::JobSystem(std::shared_ptr<core::importing::IImportStatement> importService, std::size_t workers)
    : impl_(std::make_unique<Impl>(std::move(importService), workers)) {
}

JobSystem::~JobSystem() = default;

JobSystem::JobSystem(JobSystem&&) noexcept = default;

JobSystem& JobSystem::operator=(JobSystem&&) noexcept = default;

JobId JobSystem::startImportStatement(const ImportStatementJobSpec& spec) {
    JobId id = impl_->manager.submitImportStatement(spec);

    impl_->scheduler.enqueue([this, id, spec]() {
        impl_->manager.start(id);

        try {
            if (!impl_->importService) {
                impl_->manager.fail(id, std::string(core::constants::jobs::messages::kImportServiceUnavailable));
                return;
            }
            if (spec.sourcePath.empty() || !std::filesystem::exists(spec.sourcePath)) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorSourceMissing));
                return;
            }
            if (spec.runRoot.empty()) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorRunRootMissing));
                return;
            }

            auto cancel = impl_->manager.cancelFlag(id);
            auto progressCb = [this, id](double p, const std::string& msg) {
                JobEvent ev;
                ev.jobId = id;
                ev.kind = JobKind::ImportStatement;
                ev.state = JobState::Running;
                ev.stage = JobStage::None;
                ev.progress = p;
                ev.message = msg;
                impl_->manager.publish(ev);
            };

            ImportRequest req;
            req.sourcePath = spec.sourcePath;
            req.runRoot = spec.runRoot;
            req.runIdPrefix = spec.runIdPrefix;
            req.jobId = id;
            req.progressCallback = std::move(progressCb);
            req.cancelFlag = cancel;
            req.scheduler = &impl_->scheduler;
            req.ocrLimiter = &impl_->ocrLimiter;

            auto result = impl_->importService->importStatement(req);
            if (!result.data) {
                impl_->manager.fail(id, std::string(core::constants::importing::kErrorExtractionFailed));
                return;
            }

            impl_->manager.setStatementResult(id, result.data);
            impl_->manager.setStatementTransactions(id, std::move(result.transactions));
            impl_->manager.setStatementArtifacts(id, std::move(result.artifacts));

            if (cancel && cancel->load()) {
                impl_->manager.cancel(id);
                return;
            }

            impl_->manager.finish(id);
        } catch (const std::exception& ex) {
            impl_->manager.fail(id, ex.what());
        } catch (...) {
            impl_->manager.fail(id, std::string(core::constants::jobs::messages::kUnknownError));
        }
    });

    return id;
}

SubscriptionId JobSystem::subscribe(const JobId& id, JobEventCallback cb)
{
    return impl_->manager.subscribe(id, std::move(cb));
}

void JobSystem::unsubscribe(const JobId& id, SubscriptionId subId)
{
    impl_->manager.unsubscribe(id, subId);
}

void JobSystem::cancel(const JobId& id)
{
    impl_->manager.cancel(id);
}

std::optional<JobSnapshot> JobSystem::snapshot(const JobId& id) const
{
    return impl_->manager.snapshot(id);
}

std::shared_ptr<core::domain::Statement> JobSystem::statementResult(const JobId& id) const
{
    return impl_->manager.statementResult(id);
}

std::vector<core::domain::TransactionDraft> JobSystem::statementTransactions(const JobId& id) const
{
    return impl_->manager.statementTransactions(id);
}

std::map<std::string, std::vector<uint8_t>> JobSystem::takeStatementArtifacts(const JobId& id)
{
    return impl_->manager.takeStatementArtifacts(id);
}

void JobSystem::shutdown() {
    impl_->scheduler.stop();
}

}
