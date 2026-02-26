#pragma once

#include "core/jobs/JobTypes.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Statement;

namespace core::jobs {

struct ImportStatementJobSpec {
    std::string sourcePath;
    std::string runRoot;
    std::string runIdPrefix;
};

class JobManager {
public:
    JobManager();

    JobId submitImportStatement(const ImportStatementJobSpec& spec);

    SubscriptionId subscribe(const JobId& id, JobEventCallback cb);
    void unsubscribe(const JobId& id, SubscriptionId subId);

    void cancel(const JobId& id);

    std::optional<JobSnapshot> snapshot(const JobId& id) const;

    std::shared_ptr<std::atomic<bool>> cancelFlag(const JobId& id) const;

    void setStatementResult(const JobId& id, std::shared_ptr<Statement> stmt);
    std::shared_ptr<Statement> statementResult(const JobId& id) const;

    void publish(const JobEvent& ev);
    void fail(const JobId& id, const std::string& error);
    void finish(const JobId& id);
    void start(const JobId& id);

private:
    struct JobData {
        JobSnapshot snap;
        std::shared_ptr<std::atomic<bool>> cancel;
        std::shared_ptr<Statement> statement;
        std::unordered_map<SubscriptionId, JobEventCallback> subs;
        SubscriptionId nextSub = 1;
        mutable std::mutex m;
    };

    static JobId makeJobId();

    std::unordered_map<JobId, std::shared_ptr<JobData>> jobs_;
    mutable std::mutex jobsMutex_;
};

}
