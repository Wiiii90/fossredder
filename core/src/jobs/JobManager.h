/**
 * @file core/src/jobs/JobManager.h
 * @brief Declares the private job state manager used behind `JobSystem`.
 */

#pragma once

#include "core/constants/CoreDefaults.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"
#include "core/models/TransactionDraft.h"

#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::domain {
class Statement;
}

namespace core::jobs {

class JobManager {
public:
    JobManager();

    JobId submitImportStatement(const ImportStatementJobSpec& spec);

    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);
    void unsubscribe(const JobId& id, SubscriptionId subId);

    void cancel(const JobId& id);

    std::optional<JobSnapshot> snapshot(const JobId& id) const;

    std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;

    void setStatementResult(const JobId& id, std::shared_ptr<core::domain::Statement> stmt);
    std::shared_ptr<core::domain::Statement> statementResult(const JobId& id) const;

    void setStatementTransactions(const JobId& id, std::vector<core::domain::TransactionDraft> transactions);
    std::vector<core::domain::TransactionDraft> statementTransactions(const JobId& id) const;

    void setStatementArtifacts(const JobId& id, std::map<std::string, std::vector<uint8_t>> artifacts);
    std::map<std::string, std::vector<uint8_t>> statementArtifacts(const JobId& id) const;
    std::map<std::string, std::vector<uint8_t>> takeStatementArtifacts(const JobId& id);

    void publish(const JobEvent& ev);
    void fail(const JobId& id, const std::string& error);
    void finish(const JobId& id);
    void start(const JobId& id);

private:
    void prune(std::size_t maxJobs);

    struct JobData {
        JobSnapshot snap;
        std::shared_ptr<std::atomic<bool>> cancel;
        std::shared_ptr<core::domain::Statement> statement;
        std::vector<core::domain::TransactionDraft> transactions;
        std::map<std::string, std::vector<uint8_t>> artifacts;
        std::unordered_map<SubscriptionId, JobEventCallback> subs;
        SubscriptionId nextSub = 1;
        mutable std::mutex m;
    };

    static JobId makeJobId();

    static constexpr std::size_t kMaxJobs = core::constants::jobs::kJobHistoryLimit;

    std::unordered_map<JobId, std::shared_ptr<JobData>> jobs_;
    std::deque<JobId> order_;
    mutable std::mutex jobsMutex_;
};

}
