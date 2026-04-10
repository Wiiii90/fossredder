/**
 * @file core/include/core/jobs/JobSystem.h
 * @brief Declares the import job orchestration facade used by the UI and app layer.
 */

#pragma once

#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"
#include "core/models/TransactionDraft.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::importing {
class IImportStatement;
}

namespace core::jobs {

class JobSystem {
public:
    JobSystem(std::shared_ptr<core::importing::IImportStatement> importService, std::size_t workers = 0);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) noexcept;
    JobSystem& operator=(JobSystem&&) noexcept;

    JobId startImportStatement(const ImportStatementJobSpec& spec);
    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);
    void unsubscribe(const JobId& id, SubscriptionId subId);
    void cancel(const JobId& id);
    std::optional<JobSnapshot> snapshot(const JobId& id) const;
    std::shared_ptr<core::domain::Statement> statementResult(const JobId& id) const;
    std::vector<core::domain::TransactionDraft> statementTransactions(const JobId& id) const;
    std::map<std::string, std::vector<uint8_t>> takeStatementArtifacts(const JobId& id);

    void shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
