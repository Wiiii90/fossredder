#include "core/jobs/JobSystem.h"

#include "core/constants/CoreDefaults.h"
#include "core/import/IImportStatement.h"

#include <filesystem>
#include <thread>

namespace core::jobs {

static std::size_t defaultWorkers() {
    auto hc = std::thread::hardware_concurrency();
    if (hc == 0) return core::constants::jobs::kFallbackWorkerCount;
    if (hc <= 2) return 1;
    return static_cast<std::size_t>(hc - 1);
}

static std::size_t resolveWorkerCount(std::size_t workers)
{
    return workers == 0 ? defaultWorkers() : workers;
}

JobSystem::JobSystem(std::shared_ptr<IImportStatement> importService, std::size_t workers)
    : importService_(std::move(importService))
    , manager_()
    , scheduler_(resolveWorkerCount(workers), core::constants::jobs::kQueueCapacity)
    , ocrLimiter_(std::max<std::size_t>(1, resolveWorkerCount(workers) / core::constants::jobs::kOcrWorkerDivisor)) {
}

JobId JobSystem::startImportStatement(const ImportStatementJobSpec& spec) {
    JobId id = manager_.submitImportStatement(spec);

    scheduler_.enqueue([this, id, spec]() {
        manager_.start(id);

        try {
            if (!importService_) {
                manager_.fail(id, std::string(core::constants::jobs::messages::kImportServiceUnavailable));
                return;
            }
            if (spec.sourcePath.empty() || !std::filesystem::exists(spec.sourcePath)) {
                manager_.fail(id, std::string(core::constants::importing::kErrorSourceMissing));
                return;
            }
            if (spec.runRoot.empty()) {
                manager_.fail(id, std::string(core::constants::importing::kErrorRunRootMissing));
                return;
            }

            auto cancel = manager_.cancelFlag(id);
            auto progressCb = [this, id](double p, const std::string& msg) {
                JobEvent ev;
                ev.jobId = id;
                ev.kind = JobKind::ImportStatement;
                ev.state = JobState::Running;
                ev.stage = JobStage::None;
                ev.progress = p;
                ev.message = msg;
                manager_.publish(ev);
            };

            ImportRequest req;
            req.sourcePath = spec.sourcePath;
            req.runRoot = spec.runRoot;
            req.runIdPrefix = spec.runIdPrefix;
            req.jobId = id;
            req.progressCallback = std::move(progressCb);
            req.cancelFlag = cancel;
            req.scheduler = &scheduler_;
            req.ocrLimiter = &ocrLimiter_;

            auto result = importService_->importStatement(req);
            if (!result.data) {
                manager_.fail(id, std::string(core::constants::importing::kErrorExtractionFailed));
                return;
            }

            manager_.setStatementResult(id, result.data);
            manager_.setStatementTransactions(id, std::move(result.transactions));
            manager_.setStatementArtifacts(id, std::move(result.artifacts));

            if (cancel && cancel->load()) {
                manager_.cancel(id);
                return;
            }

            manager_.finish(id);
        } catch (const std::exception& ex) {
            manager_.fail(id, ex.what());
        } catch (...) {
            manager_.fail(id, std::string(core::constants::jobs::messages::kUnknownError));
        }
    });

    return id;
}

void JobSystem::shutdown() {
    scheduler_.stop();
}

}
