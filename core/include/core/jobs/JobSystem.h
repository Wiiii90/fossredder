#pragma once

#include "core/jobs/JobManager.h"
#include "core/jobs/Scheduler.h"

#include <memory>

class ImportController;

namespace core::jobs {

class JobSystem {
public:
    JobSystem(std::shared_ptr<ImportController> importController, std::size_t workers = 0);

    JobManager& manager() noexcept { return manager_; }

    JobId startImportStatement(const ImportStatementJobSpec& spec);

    void shutdown();

    SlotLimiter& ocrLimiter() noexcept { return ocrLimiter_; }

private:
    std::shared_ptr<ImportController> importController_;

    JobManager manager_;
    Scheduler scheduler_;
    SlotLimiter ocrLimiter_;
};

}
