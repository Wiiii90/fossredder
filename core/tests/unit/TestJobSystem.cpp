#include "gtest/gtest.h"

#include "core/constants/CoreDefaults.h"
#include "core/import/IImportStatement.h"
#include "core/jobs/JobSystem.h"
#include "core/models/Statement.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

using core::domain::Statement;

namespace {

class FakeImportStatement : public IImportStatement {
public:
    ImportResult importStatement(const ImportRequest& req) override
    {
        lastRequest = req;

        ImportResult result;
        auto statement = std::make_shared<Statement>();
        statement->id = "statement-1";
        statement->name = "Imported statement";
        result.data = statement;
        return result;
    }

    ImportRequest lastRequest;
};

std::optional<core::jobs::JobSnapshot> waitForSnapshot(core::jobs::JobSystem& jobSystem,
                                                       const core::jobs::JobId& jobId)
{
    for (int attempt = 0; attempt < 100; ++attempt) {
        auto snapshot = jobSystem.snapshot(jobId);
        if (snapshot && (snapshot->state == core::jobs::JobState::Finished
                         || snapshot->state == core::jobs::JobState::Failed
                         || snapshot->state == core::jobs::JobState::Canceled)) {
            return snapshot;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return jobSystem.snapshot(jobId);
}

}

TEST(JobSystemTests, StartImportStatement_finishes_and_stores_statement_result)
{
    auto importService = std::make_shared<FakeImportStatement>();
    core::jobs::JobSystem jobSystem(importService, 1);

    const auto sourcePath = (std::filesystem::temp_directory_path() / "fossredder-job-system-test.pdf").string();
    {
        std::ofstream sourceFile(sourcePath);
        sourceFile << "test";
    }

    core::jobs::ImportStatementJobSpec spec;
    spec.sourcePath = sourcePath;
    spec.runRoot = "artifacts";
    spec.runIdPrefix = "job";

    const auto jobId = jobSystem.startImportStatement(spec);
    const auto snapshot = waitForSnapshot(jobSystem, jobId);

    ASSERT_TRUE(snapshot.has_value());
    EXPECT_EQ(snapshot->state, core::jobs::JobState::Finished);
    EXPECT_EQ(snapshot->message, std::string(core::constants::jobs::messages::kFinished));

    auto statement = jobSystem.statementResult(jobId);
    ASSERT_TRUE(statement);
    EXPECT_EQ(statement->id, std::string("statement-1"));
    EXPECT_EQ(importService->lastRequest.jobId, jobId);
    EXPECT_EQ(importService->lastRequest.sourcePath, spec.sourcePath);

    jobSystem.shutdown();
    std::filesystem::remove(sourcePath);
}

TEST(JobSystemTests, StartImportStatement_fails_when_import_service_is_missing)
{
    core::jobs::JobSystem jobSystem(nullptr, 1);

    core::jobs::ImportStatementJobSpec spec;
    spec.sourcePath = "statement.pdf";
    spec.runRoot = "artifacts";
    spec.runIdPrefix = "job";

    const auto jobId = jobSystem.startImportStatement(spec);
    const auto snapshot = waitForSnapshot(jobSystem, jobId);

    ASSERT_TRUE(snapshot.has_value());
    EXPECT_EQ(snapshot->state, core::jobs::JobState::Failed);
    EXPECT_EQ(snapshot->error, std::string(core::constants::jobs::messages::kImportServiceUnavailable));

    jobSystem.shutdown();
}
