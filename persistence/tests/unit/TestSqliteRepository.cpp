/**
 * @file persistence/tests/unit/TestSqliteRepository.cpp
 * @brief Unit tests that execute CRUD/query paths across SQLite repositories.
 */

#include "gtest/gtest.h"

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/domain/entities/Transaction.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/ports/repositories/IActorRepository.h"
#include "core/ports/repositories/IContractRepository.h"
#include "core/ports/repositories/IPropertyRepository.h"
#include "core/ports/repositories/IStatementRepository.h"
#include "core/ports/repositories/ITransactionRepository.h"
#include "core/ports/repositories/IAnalysisRepository.h"
#include "core/ports/repositories/IAnnualRepository.h"
#include "core/ports/repositories/IStatementDraftRepository.h"
#include "core/ports/repositories/ITransactionDraftRepository.h"
#include "persistence/Factory.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

namespace {

class TempDatabaseFile {
public:
    TempDatabaseFile()
    {
        const auto uniqueId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        path_ = std::filesystem::temp_directory_path() /
                ("fossredder-sqlite-repository-" + uniqueId + ".db");
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

std::shared_ptr<core::domain::Actor> makeActor(const std::string& id, const std::string& name)
{
    auto actor = std::make_shared<core::domain::Actor>();
    actor->id = id;
    actor->name = name;
    actor->aliases = {core::domain::Alias{.value = name + " alias"}};
    return actor;
}

std::shared_ptr<core::domain::Property> makeProperty(const std::string& id, const std::string& name)
{
    auto property = std::make_shared<core::domain::Property>();
    property->id = id;
    property->name = name;
    property->aliases = {core::domain::Alias{.value = name + " alias"}};
    return property;
}

std::shared_ptr<core::domain::Contract> makeContract(const std::string& id,
                                                      const std::vector<std::string>& actorIds,
                                                      const std::vector<std::string>& propertyIds)
{
    auto contract = std::make_shared<core::domain::Contract>();
    contract->id = id;
    contract->name = "Contract " + id;
    contract->type = "rental";
    contract->actorIds = actorIds;
    contract->propertyIds = propertyIds;
    contract->aliases = {core::domain::Alias{.value = "contract alias " + id}};
    return contract;
}

std::shared_ptr<core::domain::Statement> makeStatement(const std::string& id, const std::string& name)
{
    auto statement = std::make_shared<core::domain::Statement>();
    statement->id = id;
    statement->name = name;
    return statement;
}

std::shared_ptr<core::domain::Transaction> makeTransaction(const std::string& id,
                                                           const std::string& statementId,
                                                           const std::string& contractId)
{
    auto tx = std::make_shared<core::domain::Transaction>();
    tx->id = id;
    tx->name = "Transaction " + id;
    tx->bookingDate = "2025-01-01";
    tx->valuta = "2025-01-02";
    tx->amount = 123.45;
    tx->description = "transaction-desc";
    tx->statementId = statementId;
    tx->contractId = contractId;
    tx->actorId = "actor-1";
    tx->status = core::domain::Transaction::Status::Unverified;
    tx->allocatable = true;
    tx->propertyIds = {"property-1"};
    return tx;
}

std::shared_ptr<core::domain::Analysis> makeAnalysis(const std::string& id)
{
    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->id = id;
    analysis->name = "Analysis " + id;
    analysis->type = "tab";
    analysis->configJson = R"({"chart":"line"})";
    analysis->filterSpec = "type=rental";
    analysis->createdAt = "2025-01-01T00:00:00Z";
    analysis->updatedAt = "2025-01-02T00:00:00Z";
    return analysis;
}

std::shared_ptr<core::domain::Annual> makeAnnual(const std::string& id)
{
    auto annual = std::make_shared<core::domain::Annual>();
    annual->id = id;
    annual->year = 2025;
    annual->transactionIds = {"tx-1", "tx-2"};
    annual->assignedAnalysisIds = {"analysis-1"};
    annual->verificationState = core::domain::Annual::VerificationState::Verified;
    annual->createdAt = "2025-01-01T00:00:00Z";
    annual->updatedAt = "2025-01-02T00:00:00Z";
    return annual;
}

} // namespace

TEST(SqliteRepositoryTests, ActorRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());
    const auto repo = createSqliteActorRepository(db);

    auto actor = makeActor("actor-1", "Alice");
    repo->addActor(actor);

    ASSERT_EQ(repo->getActors().size(), 1u);
    ASSERT_TRUE(repo->getActorById("actor-1").has_value());

    actor->name = "Alice Updated";
    repo->updateActor(actor);
    EXPECT_EQ((*repo->getActorById("actor-1"))->name, "Alice Updated");

    auto actor2 = makeActor("actor-2", "Bob");
    repo->upsertActor(actor2);
    EXPECT_EQ(repo->getActors().size(), 2u);

    repo->removeActor("actor-2");
    EXPECT_FALSE(repo->getActorById("actor-2").has_value());

    repo->clearActors();
    EXPECT_TRUE(repo->getActors().empty());
}

TEST(SqliteRepositoryTests, PropertyRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());
    const auto repo = createSqlitePropertyRepository(db);

    auto property = makeProperty("property-1", "Main Building");
    repo->addProperty(property);

    ASSERT_EQ(repo->getProperties().size(), 1u);
    ASSERT_TRUE(repo->getPropertyById("property-1").has_value());

    property->name = "Main Building Updated";
    repo->updateProperty(property);
    EXPECT_EQ((*repo->getPropertyById("property-1"))->name, "Main Building Updated");

    auto property2 = makeProperty("property-2", "Second Building");
    repo->upsertProperty(property2);
    EXPECT_EQ(repo->getProperties().size(), 2u);

    repo->removeProperty("property-2");
    EXPECT_FALSE(repo->getPropertyById("property-2").has_value());

    repo->clearProperties();
    EXPECT_TRUE(repo->getProperties().empty());
}

TEST(SqliteRepositoryTests, ContractRepositoryExecutesCrudAndRelationQueries)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());

    const auto actorRepo = createSqliteActorRepository(db);
    const auto propertyRepo = createSqlitePropertyRepository(db);
    const auto contractRepo = createSqliteContractRepository(db);

    actorRepo->addActor(makeActor("actor-1", "Alice"));
    actorRepo->addActor(makeActor("actor-2", "Bob"));
    propertyRepo->addProperty(makeProperty("property-1", "Main"));
    propertyRepo->addProperty(makeProperty("property-2", "Side"));

    auto contract = makeContract("contract-1", {"actor-1"}, {"property-1"});
    contractRepo->addContract(contract);

    ASSERT_EQ(contractRepo->getContracts().size(), 1u);
    ASSERT_TRUE(contractRepo->getContractById("contract-1").has_value());
    ASSERT_EQ(contractRepo->getContractsForActor("actor-1").size(), 1u);
    ASSERT_EQ(contractRepo->getContractsForProperty("property-1").size(), 1u);
    EXPECT_EQ(contractRepo->getActorIdsForContract("contract-1"), (std::vector<std::string>{"actor-1"}));
    EXPECT_EQ(contractRepo->getPropertyIdsForContract("contract-1"), (std::vector<std::string>{"property-1"}));

    contract->name = "Contract Updated";
    contract->actorIds = {"actor-1", "actor-2"};
    contract->propertyIds = {"property-1", "property-2"};
    contractRepo->updateContract(contract);
    EXPECT_EQ((*contractRepo->getContractById("contract-1"))->name, "Contract Updated");

    auto contract2 = makeContract("contract-2", {"actor-2"}, {"property-2"});
    contractRepo->upsertContract(contract2);
    EXPECT_EQ(contractRepo->getContracts().size(), 2u);

    contractRepo->removeContract("contract-2");
    EXPECT_FALSE(contractRepo->getContractById("contract-2").has_value());

    contractRepo->clearContracts();
    EXPECT_TRUE(contractRepo->getContracts().empty());
}

TEST(SqliteRepositoryTests, StatementRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());

    const auto statementRepo = createSqliteStatementRepository(db);

    statementRepo->addStatement(makeStatement("statement-1", "January"));

    auto statement = makeStatement("statement-2", "February");
    statementRepo->upsertStatement(statement);
    ASSERT_EQ(statementRepo->getStatements().size(), 2u);
    ASSERT_TRUE(statementRepo->getStatementById("statement-1").has_value());

    statement->name = "February Updated";
    statementRepo->updateStatement(statement);
    EXPECT_EQ((*statementRepo->getStatementById("statement-2"))->name, "February Updated");

    statementRepo->removeStatement("statement-2");
    EXPECT_FALSE(statementRepo->getStatementById("statement-2").has_value());

    statementRepo->clearStatements();
    EXPECT_TRUE(statementRepo->getStatements().empty());
}

TEST(SqliteRepositoryTests, TransactionRepositoryExecutesCrudAndContractAssignments)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());

    const auto actorRepo = createSqliteActorRepository(db);
    const auto propertyRepo = createSqlitePropertyRepository(db);
    const auto statementRepo = createSqliteStatementRepository(db);
    const auto contractRepo = createSqliteContractRepository(db);
    const auto transactionRepo = createSqliteTransactionRepository(db);

    actorRepo->addActor(makeActor("actor-1", "Alice"));
    propertyRepo->addProperty(makeProperty("property-1", "Main"));
    statementRepo->addStatement(makeStatement("statement-1", "January"));
    statementRepo->addStatement(makeStatement("statement-2", "February"));
    contractRepo->addContract(makeContract("contract-1", {"actor-1"}, {"property-1"}));

    auto tx = makeTransaction("tx-1", "statement-1", "contract-1");
    transactionRepo->addTransaction(tx);

    ASSERT_EQ(transactionRepo->getTransactions().size(), 1u);
    ASSERT_TRUE(transactionRepo->getTransactionById("tx-1").has_value());
    ASSERT_EQ(transactionRepo->getTransactionsForContract("contract-1").size(), 1u);

    tx->amount = 777.0;
    tx->status = core::domain::Transaction::Status::Verified;
    transactionRepo->updateTransaction(tx);
    EXPECT_DOUBLE_EQ((*transactionRepo->getTransactionById("tx-1"))->amount, 777.0);

    auto tx2 = makeTransaction("tx-2", "statement-2", "");
    transactionRepo->upsertTransaction(tx2);
    transactionRepo->assignTransactionsToContract("contract-1", {"tx-1", "tx-2"});
    EXPECT_EQ(transactionRepo->getTransactionsForContract("contract-1").size(), 2u);

    transactionRepo->removeTransaction("tx-2");
    EXPECT_FALSE(transactionRepo->getTransactionById("tx-2").has_value());

    transactionRepo->clearTransactions();
    EXPECT_TRUE(transactionRepo->getTransactions().empty());
}

TEST(SqliteRepositoryTests, AnalysisRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());

    const auto analysisRepo = createSqliteAnalysisRepository(db);

    auto analysis = makeAnalysis("analysis-1");
    analysisRepo->addAnalysis(analysis);
    ASSERT_EQ(analysisRepo->getAnalyses().size(), 1u);
    ASSERT_TRUE(analysisRepo->getAnalysisById("analysis-1").has_value());

    analysis->name = "Analysis Updated";
    analysisRepo->updateAnalysis(analysis);
    EXPECT_EQ((*analysisRepo->getAnalysisById("analysis-1"))->name, "Analysis Updated");

    analysisRepo->upsertAnalysis(makeAnalysis("analysis-2"));
    EXPECT_EQ(analysisRepo->getAnalyses().size(), 2u);

    analysisRepo->removeAnalysis("analysis-2");
    EXPECT_FALSE(analysisRepo->getAnalysisById("analysis-2").has_value());

    analysisRepo->clearAnalyses();
    EXPECT_TRUE(analysisRepo->getAnalyses().empty());
}

TEST(SqliteRepositoryTests, AnnualRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());

    const auto annualRepo = createSqliteAnnualRepository(db);

    auto annual = makeAnnual("annual-2025");
    annualRepo->addAnnual(annual);
    ASSERT_EQ(annualRepo->getAnnuals().size(), 1u);
    ASSERT_TRUE(annualRepo->getAnnualById("annual-2025").has_value());

    annual->verificationState = core::domain::Annual::VerificationState::Locked;
    annualRepo->updateAnnual(annual);
    EXPECT_EQ((*annualRepo->getAnnualById("annual-2025"))->verificationState,
              core::domain::Annual::VerificationState::Locked);

    annualRepo->upsertAnnual(makeAnnual("annual-2026"));
    EXPECT_EQ(annualRepo->getAnnuals().size(), 2u);

    annualRepo->removeAnnual("annual-2026");
    EXPECT_FALSE(annualRepo->getAnnualById("annual-2026").has_value());

    annualRepo->clearAnnuals();
    EXPECT_TRUE(annualRepo->getAnnuals().empty());
}

TEST(SqliteRepositoryTests, StatementDraftRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());
    const auto statementDraftRepo = createSqliteStatementDraftRepository(db);

    auto statementDraft = std::make_shared<core::application::importing::draft::StatementDraft>();
    statementDraft->id = "statement-draft-1";
    statementDraft->name = "Imported Statement";
    statementDraftRepo->addStatementDraft(statementDraft);

    const auto allDrafts = statementDraftRepo->getStatementDrafts();
    ASSERT_EQ(allDrafts.size(), 1u);

    const auto loadedStatement = statementDraftRepo->getStatementDraftById("statement-draft-1");
    ASSERT_TRUE(loadedStatement.has_value());
    ASSERT_TRUE(*loadedStatement);
    EXPECT_EQ((*loadedStatement)->name, "Imported Statement");

    statementDraft->name = "Imported Statement Updated";
    statementDraftRepo->updateStatementDraft(statementDraft);
    EXPECT_EQ((*statementDraftRepo->getStatementDraftById("statement-draft-1"))->name,
              "Imported Statement Updated");

    auto secondDraft = std::make_shared<core::application::importing::draft::StatementDraft>();
    secondDraft->id = "statement-draft-2";
    secondDraft->name = "Second Draft";
    statementDraftRepo->upsertStatementDraft(secondDraft);
    EXPECT_EQ(statementDraftRepo->getStatementDrafts().size(), 2u);

    statementDraftRepo->removeStatementDraft("statement-draft-2");
    EXPECT_FALSE(statementDraftRepo->getStatementDraftById("statement-draft-2").has_value());

    statementDraftRepo->clearStatementDrafts();
    EXPECT_TRUE(statementDraftRepo->getStatementDrafts().empty());
}

TEST(SqliteRepositoryTests, TransactionDraftRepositoryExecutesCrudAndLookup)
{
    TempDatabaseFile tempDb;
    const auto db = createSqliteDb(tempDb.path().string());
    const auto statementDraftRepo = createSqliteStatementDraftRepository(db);
    const auto transactionDraftRepo = createSqliteTransactionDraftRepository(db);

    auto parentStatement = std::make_shared<core::application::importing::draft::StatementDraft>();
    parentStatement->id = "statement-draft-1";
    parentStatement->name = "Parent Statement";
    statementDraftRepo->addStatementDraft(parentStatement);

    auto tx1 = std::make_shared<core::application::importing::draft::TransactionDraft>();
    tx1->id = "tx-draft-1";
    tx1->statementDraftId = "statement-draft-1";
    tx1->position = 0;
    tx1->name = "Draft Tx 1";
    tx1->bookingDate = "2025-01-01";
    tx1->amount = 50.0;
    tx1->metadata = "meta-1";
    transactionDraftRepo->addTransactionDraft(tx1);

    const auto loadedTx1 = transactionDraftRepo->getTransactionDraftById("tx-draft-1");
    ASSERT_TRUE(loadedTx1.has_value());
    ASSERT_TRUE(*loadedTx1);
    EXPECT_EQ((*loadedTx1)->name, "Draft Tx 1");

    tx1->name = "Draft Tx 1 Updated";
    transactionDraftRepo->updateTransactionDraft(tx1);
    EXPECT_EQ((*transactionDraftRepo->getTransactionDraftById("tx-draft-1"))->name, "Draft Tx 1 Updated");

    auto tx2 = std::make_shared<core::application::importing::draft::TransactionDraft>();
    tx2->id = "tx-draft-2";
    tx2->statementDraftId = "statement-draft-1";
    tx2->position = 1;
    tx2->name = "Draft Tx 2";
    tx2->bookingDate = "2025-01-02";
    tx2->amount = 75.0;
    tx2->metadata = "meta-2";

    transactionDraftRepo->upsertTransactionDraft(tx2);

    const auto loadedDrafts = transactionDraftRepo->getTransactionDrafts();
    ASSERT_EQ(loadedDrafts.size(), 2u);
    EXPECT_EQ(loadedDrafts[0]->name, "Draft Tx 1 Updated");
    EXPECT_EQ(loadedDrafts[1]->name, "Draft Tx 2");

    transactionDraftRepo->removeTransactionDraft("tx-draft-2");
    EXPECT_FALSE(transactionDraftRepo->getTransactionDraftById("tx-draft-2").has_value());

    transactionDraftRepo->clearTransactionDrafts();
    EXPECT_TRUE(transactionDraftRepo->getTransactionDrafts().empty());
}
