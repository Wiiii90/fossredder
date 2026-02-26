#include "core/jobs/JobSystem.h"

#include "core/controllers/ImportController.h"

#include <thread>

namespace core::jobs {

static std::size_t defaultWorkers() {
    auto hc = std::thread::hardware_concurrency();
    if (hc == 0) return 4;
    if (hc <= 2) return 1;
    return static_cast<std::size_t>(hc - 1);
}

JobSystem::JobSystem(std::shared_ptr<ImportController> importController, std::size_t workers)
    : importController_(std::move(importController))
    , manager_()
    , scheduler_(workers == 0 ? defaultWorkers() : workers, /*queueCapacity*/128)
    , ocrLimiter_(std::max<std::size_t>(1, (workers == 0 ? defaultWorkers() : workers) / 2)) {
}

JobId JobSystem::startImportStatement(const ImportStatementJobSpec& spec) {
    JobId id = manager_.submitImportStatement(spec);

    scheduler_.enqueue([this, id, spec]() {
        manager_.start(id);

        try {
            if (!importController_) {
                manager_.fail(id, "Import controller not available");
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

            auto stmt = importController_->import(ImportController::ImportType::Statement,
                                                  spec.sourcePath,
                                                  spec.runRoot,
                                                  spec.runIdPrefix,
                                                  progressCb,
                                                  cancel,
                                                  &scheduler_,
                                                  &ocrLimiter_,
                                                  id);

            manager_.setStatementResult(id, stmt.data);
            manager_.setStatementArtifacts(id, std::move(stmt.artifacts));

            if (cancel && cancel->load()) {
                manager_.cancel(id);
                return;
            }

            manager_.finish(id);
        } catch (const std::exception& ex) {
            manager_.fail(id, ex.what());
        } catch (...) {
            manager_.fail(id, "Unknown error");
        }
    });

    return id;
}

void JobSystem::shutdown() {
    scheduler_.stop();
}

}
