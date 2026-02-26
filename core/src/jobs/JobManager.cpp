#include "core/jobs/JobManager.h"

#include "core/utils/UniqId.h"

#include <utility>

namespace core::jobs {

JobManager::JobManager() = default;

JobId JobManager::makeJobId() {
    return utils::makeUniqId();
}

JobId JobManager::submitImportStatement(const ImportStatementJobSpec& spec) {
    auto data = std::make_shared<JobData>();
    data->snap.jobId = makeJobId();
    data->snap.kind = JobKind::ImportStatement;
    data->snap.state = JobState::Pending;
    data->snap.stage = JobStage::None;
    data->snap.progress = 0.0;
    data->snap.message = "Queued";
    data->cancel = std::make_shared<std::atomic<bool>>(false);

    (void)spec;

    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        jobs_.emplace(data->snap.jobId, data);
        order_.push_back(data->snap.jobId);
    }
    prune(kMaxJobs);
    return data->snap.jobId;
}

SubscriptionId JobManager::subscribe(const JobId& id, JobEventCallback cb) {
    if (!cb) return 0;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return 0;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    SubscriptionId sid = job->nextSub++;
    job->subs.emplace(sid, std::move(cb));
    return sid;
}

void JobManager::unsubscribe(const JobId& id, SubscriptionId subId) {
    if (subId == 0) return;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    job->subs.erase(subId);
}

void JobManager::cancel(const JobId& id) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    if (job->cancel) {
        try { job->cancel->store(true); } catch (...) {}
    }

    JobEvent ev;
    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Canceled;
        job->snap.message = "Canceled";
        ev.jobId = job->snap.jobId;
        ev.kind = job->snap.kind;
        ev.state = job->snap.state;
        ev.stage = job->snap.stage;
        ev.progress = job->snap.progress;
        ev.message = job->snap.message;
        ev.pageIndex = job->snap.pageIndex;
        ev.pageCount = job->snap.pageCount;
    }
    publish(ev);

    prune(kMaxJobs);
}

std::optional<JobSnapshot> JobManager::snapshot(const JobId& id) const {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return std::nullopt;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    return job->snap;
}

std::shared_ptr<std::atomic<bool>> JobManager::cancelFlag(const JobId& id) const {
    std::lock_guard<std::mutex> g(jobsMutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end()) return nullptr;
    return it->second->cancel;
}

void JobManager::setStatementResult(const JobId& id, std::shared_ptr<Statement> stmt) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    job->statement = std::move(stmt);
}

std::shared_ptr<Statement> JobManager::statementResult(const JobId& id) const {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return nullptr;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    return job->statement;
}

void JobManager::setStatementArtifacts(const JobId& id, std::map<std::string, std::vector<uint8_t>> artifacts) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    job->artifacts = std::move(artifacts);
}

std::map<std::string, std::vector<uint8_t>> JobManager::statementArtifacts(const JobId& id) const {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return {};
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    return job->artifacts;
}

std::map<std::string, std::vector<uint8_t>> JobManager::takeStatementArtifacts(const JobId& id) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return {};
        job = it->second;
    }

    std::lock_guard<std::mutex> g(job->m);
    auto out = std::move(job->artifacts);
    job->artifacts.clear();
    return out;
}

void JobManager::publish(const JobEvent& ev) {
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(ev.jobId);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    std::vector<JobEventCallback> cbs;
    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.kind = ev.kind;
        job->snap.state = ev.state;
        job->snap.stage = ev.stage;
        job->snap.progress = ev.progress;
        if (!ev.message.empty()) job->snap.message = ev.message;
        job->snap.pageIndex = ev.pageIndex;
        job->snap.pageCount = ev.pageCount;

        cbs.reserve(job->subs.size());
        for (auto& kv : job->subs) cbs.push_back(kv.second);
    }

    for (auto& cb : cbs) {
        try { cb(ev); } catch (...) {}
    }
}

void JobManager::start(const JobId& id) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Running;
        job->snap.message = "Running";
        ev.jobId = job->snap.jobId;
        ev.kind = job->snap.kind;
        ev.state = job->snap.state;
        ev.stage = job->snap.stage;
        ev.progress = job->snap.progress;
        ev.message = job->snap.message;
        ev.pageIndex = job->snap.pageIndex;
        ev.pageCount = job->snap.pageCount;
    }

    publish(ev);

    prune(kMaxJobs);
}

void JobManager::finish(const JobId& id) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Finished;
        job->snap.progress = 1.0;
        job->snap.message = "Finished";
        ev.jobId = job->snap.jobId;
        ev.kind = job->snap.kind;
        ev.state = job->snap.state;
        ev.stage = job->snap.stage;
        ev.progress = job->snap.progress;
        ev.message = job->snap.message;
        ev.pageIndex = job->snap.pageIndex;
        ev.pageCount = job->snap.pageCount;
    }

    publish(ev);

    prune(kMaxJobs);
}

void JobManager::prune(std::size_t maxJobs) {
    if (maxJobs < 1) maxJobs = 1;

    std::lock_guard<std::mutex> g(jobsMutex_);
    if (jobs_.size() <= maxJobs) return;

    // Attempt to evict terminal jobs without subscribers in FIFO order.
    // If none are evictable, keep them (avoid deleting running/subscribed jobs).
    std::size_t scanned = 0;
    const std::size_t limit = order_.size();
    while (jobs_.size() > maxJobs && scanned < limit && !order_.empty()) {
        JobId id = std::move(order_.front());
        order_.pop_front();
        ++scanned;

        auto it = jobs_.find(id);
        if (it == jobs_.end()) continue;
        auto job = it->second;
        if (!job) {
            jobs_.erase(it);
            continue;
        }

        bool evictable = false;
        {
            std::lock_guard<std::mutex> gj(job->m);
            const auto st = job->snap.state;
            const bool terminal = (st == JobState::Finished || st == JobState::Failed || st == JobState::Canceled);
            evictable = terminal && job->subs.empty();
        }

        if (evictable) {
            jobs_.erase(it);
            continue;
        }

        // keep it in rotation
        order_.push_back(std::move(id));
    }
}

void JobManager::fail(const JobId& id, const std::string& error) {
    JobEvent ev;
    std::shared_ptr<JobData> job;
    {
        std::lock_guard<std::mutex> g(jobsMutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end()) return;
        job = it->second;
    }

    {
        std::lock_guard<std::mutex> g(job->m);
        job->snap.state = JobState::Failed;
        job->snap.error = error;
        job->snap.message = "Failed";
        ev.jobId = job->snap.jobId;
        ev.kind = job->snap.kind;
        ev.state = job->snap.state;
        ev.stage = job->snap.stage;
        ev.progress = job->snap.progress;
        ev.message = error.empty() ? job->snap.message : error;
        ev.pageIndex = job->snap.pageIndex;
        ev.pageCount = job->snap.pageCount;
    }

    publish(ev);
}

}
