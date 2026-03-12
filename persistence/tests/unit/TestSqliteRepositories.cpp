/**
 * @file persistence/tests/unit/TestSqliteRepositories.cpp
 * @brief Unit-style repository tests for the SQLite persistence layer.
 */

#include "gtest/gtest.h"

#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Statement.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "persistence/Factory.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

using core::domain::Analysis;
using core::domain::Annual;
using core::domain::Statement;

namespace {

class TempDatabaseFile {
public:
    TempDatabaseFile()
    {
        const auto uniqueId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        path_ = std::filesystem::temp_directory_path() /
                ("fossredder-persistence-tests-" + uniqueId + ".db");
    }

    ~TempDatabaseFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(SqliteStatementRepositoryTests, UpsertUpdateAndRemoveRoundTripAStatement)
{
    TempDatabaseFile tempDb;
    auto db = createSqliteDb(tempDb.path().string());
    auto repo = createSqliteStatementRepository(db);

    auto statement = std::make_shared<Statement>();
    statement->id = "statement-1";
    statement->name = "January Statement";

    repo->upsertStatement(statement);

    auto loaded = repo->getStatementById(statement->id);
    ASSERT_TRUE(loaded.has_value());
    ASSERT_TRUE(*loaded);
    EXPECT_EQ((*loaded)->name, "January Statement");

    statement->name = "Updated Statement";
    repo->upsertStatement(statement);

    loaded = repo->getStatementById(statement->id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->name, "Updated Statement");

    repo->removeStatement(statement->id);
    EXPECT_FALSE(repo->getStatementById(statement->id).has_value());
}

TEST(SqliteAnalysisRepositoryTests, UpsertUpdateAndRemoveRoundTripAnAnalysis)
{
    TempDatabaseFile tempDb;
    auto db = createSqliteDb(tempDb.path().string());
    auto repo = createSqliteAnalysisRepository(db);

    auto analysis = std::make_shared<Analysis>();
    analysis->id = "analysis-1";
    analysis->name = "Annual Summary";
    analysis->type = "tab";
    analysis->configJson = R"({"window":"year"})";
    analysis->filterSpec = "contract.type=rent";
    analysis->createdAt = "2025-01-01T00:00:00Z";
    analysis->updatedAt = "2025-01-02T00:00:00Z";
    analysis->schemaVersion = 3;

    repo->upsertAnalysis(analysis);

    auto loaded = repo->getAnalysisById(analysis->id);
    ASSERT_TRUE(loaded.has_value());
    ASSERT_TRUE(*loaded);
    EXPECT_EQ((*loaded)->type, "tab");
    EXPECT_EQ((*loaded)->filterSpec, "contract.type=rent");
    EXPECT_EQ((*loaded)->schemaVersion, 3);

    analysis->type = "plot";
    analysis->updatedAt = "2025-01-03T00:00:00Z";
    repo->upsertAnalysis(analysis);

    loaded = repo->getAnalysisById(analysis->id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->type, "plot");
    EXPECT_EQ((*loaded)->updatedAt, "2025-01-03T00:00:00Z");

    repo->removeAnalysis(analysis->id);
    EXPECT_FALSE(repo->getAnalysisById(analysis->id).has_value());
}

TEST(SqliteAnnualRepositoryTests, UpsertUpdateAndRemoveRoundTripAnAnnualAggregate)
{
    TempDatabaseFile tempDb;
    auto db = createSqliteDb(tempDb.path().string());
    auto repo = createSqliteAnnualRepository(db);

    auto annual = std::make_shared<Annual>();
    annual->id = "annual-2025";
    annual->year = 2025;
    annual->transactionIds = {"tx-1", "tx-2"};
    annual->assignedAnalysisIds = {"analysis-1", "analysis-2"};
    annual->verificationState = Annual::VerificationState::Verified;
    annual->createdAt = "2025-01-01T00:00:00Z";
    annual->updatedAt = "2025-01-02T00:00:00Z";
    annual->schemaVersion = 2;

    repo->upsertAnnual(annual);

    auto loaded = repo->getAnnualById(annual->id);
    ASSERT_TRUE(loaded.has_value());
    ASSERT_TRUE(*loaded);
    const std::vector<std::string> expectedTransactionIds{"tx-1", "tx-2"};
    const std::vector<std::string> expectedAnalysisIds{"analysis-1", "analysis-2"};
    EXPECT_EQ((*loaded)->year, 2025);
    EXPECT_EQ((*loaded)->transactionIds, expectedTransactionIds);
    EXPECT_EQ((*loaded)->assignedAnalysisIds, expectedAnalysisIds);
    EXPECT_EQ((*loaded)->verificationState, Annual::VerificationState::Verified);

    annual->verificationState = Annual::VerificationState::Locked;
    annual->assignedAnalysisIds.push_back("analysis-3");
    repo->upsertAnnual(annual);

    loaded = repo->getAnnualById(annual->id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ((*loaded)->verificationState, Annual::VerificationState::Locked);
    const std::vector<std::string> expectedUpdatedAnalysisIds{
        "analysis-1", "analysis-2", "analysis-3"};
    EXPECT_EQ((*loaded)->assignedAnalysisIds, expectedUpdatedAnalysisIds);

    repo->removeAnnual(annual->id);
    EXPECT_FALSE(repo->getAnnualById(annual->id).has_value());
}
