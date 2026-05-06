/**
 * @file core/tests/TestAppStateManager.cpp
 * @brief Unit tests for AppStateManager (UNIT).
 */

#include "gtest/gtest.h"

#include "core/application/AppStateManager.h"
#include "core/models/Actor.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/ExportLog.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IExportLogRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "core/storage/RepositoryBundle.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using core::application::AppStateManager;
using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::ExportLog;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;

namespace {

class FakeExportLogRepository : public IExportLogRepository {
public:
    std::vector<std::shared_ptr<ExportLog>> logs;
    bool upsertCalled = false;

    void addExportLog(const std::shared_ptr<ExportLog>& log) override { logs.push_back(log); }
    std::vector<std::shared_ptr<ExportLog>> getExportLogs() const override { return logs; }
    std::optional<std::shared_ptr<ExportLog>> getExportLogById(const std::string& id) const override {
        for (const auto& log : logs) {
            if (log && log->id == id) return log;
        }
        return std::nullopt;
    }
    void removeExportLog(const std::string&) override {}
    void updateExportLog(const std::shared_ptr<ExportLog>&) override {}
    void upsertExportLog(const std::shared_ptr<ExportLog>& log) override { upsertCalled = true; logs.push_back(log); }
    void clearExportLogs() override { logs.clear(); }
};

class FakeActorRepository : public IActorRepository {
public:
    std::vector<std::shared_ptr<Actor>> actors;
    bool upsertCalled = false;

    void addActor(const std::shared_ptr<Actor>& actor) override { actors.push_back(actor); }
    std::vector<std::shared_ptr<Actor>> getActors() const override { return actors; }
    std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const override {
        for (const auto& actor : actors) {
            if (actor && actor->id == id) return actor;
        }
        return std::nullopt;
    }
    void removeActor(const std::string&) override {}
    void updateActor(const std::shared_ptr<Actor>&) override {}
    void upsertActor(const std::shared_ptr<Actor>& actor) override { upsertCalled = true; (void)actor; }
    void clearActors() override { actors.clear(); }
};

class FakePropertyRepository : public IPropertyRepository {
public:
    std::vector<std::shared_ptr<Property>> properties;
    bool upsertCalled = false;

    void addProperty(const std::shared_ptr<Property>& property) override { properties.push_back(property); }
    std::vector<std::shared_ptr<Property>> getProperties() const override { return properties; }
    std::optional<std::shared_ptr<Property>> getPropertyById(const std::string& id) const override {
        for (const auto& property : properties) {
            if (property && property->id == id) return property;
        }
        return std::nullopt;
    }
    void removeProperty(const std::string&) override {}
    void updateProperty(const std::shared_ptr<Property>&) override {}
    void upsertProperty(const std::shared_ptr<Property>& property) override { upsertCalled = true; (void)property; }
    void clearProperties() override { properties.clear(); }
};

class FakeContractRepository : public IContractRepository {
public:
    std::vector<std::shared_ptr<Contract>> contracts;
    bool upsertCalled = false;
    std::shared_ptr<Contract> lastUpserted;

    void addContract(const std::shared_ptr<Contract>& contract) override { contracts.push_back(contract); }
    std::vector<std::shared_ptr<Contract>> getContracts() const override { return contracts; }
    std::optional<std::shared_ptr<Contract>> getContractById(const std::string& id) const override {
        for (const auto& contract : contracts) {
            if (contract && contract->id == id) return contract;
        }
        return std::nullopt;
    }
    void removeContract(const std::string&) override {}
    void updateContract(const std::shared_ptr<Contract>&) override {}
    void upsertContract(const std::shared_ptr<Contract>& contract) override { upsertCalled = true; lastUpserted = contract; }
    void clearContracts() override { contracts.clear(); }
    std::vector<std::shared_ptr<Contract>> getContractsForActor(const std::string& actorId) const override {
        std::vector<std::shared_ptr<Contract>> out;
        for (const auto& contract : contracts) {
            if (!contract) continue;
            if (std::find(contract->actorIds.begin(), contract->actorIds.end(), actorId) != contract->actorIds.end()) {
                out.push_back(contract);
            }
        }
        return out;
    }
    std::vector<std::shared_ptr<Contract>> getContractsForProperty(const std::string& propertyId) const override {
        std::vector<std::shared_ptr<Contract>> out;
        for (const auto& contract : contracts) {
            if (!contract) continue;
            if (std::find(contract->propertyIds.begin(), contract->propertyIds.end(), propertyId) != contract->propertyIds.end()) {
                out.push_back(contract);
            }
        }
        return out;
    }
    std::vector<std::string> getActorIdsForContract(const std::string& contractId) const override {
        const auto contract = getContractById(contractId);
        return contract && *contract ? (*contract)->actorIds : std::vector<std::string>{};
    }
    std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const override {
        const auto contract = getContractById(contractId);
        return contract && *contract ? (*contract)->propertyIds : std::vector<std::string>{};
    }
};

class FakeStatementRepository : public IStatementRepository {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    bool upsertCalled = false;

    void addStatement(const std::shared_ptr<Statement>& statement) override { statements.push_back(statement); }
    std::vector<std::shared_ptr<Statement>> getStatements() const override { return statements; }
    std::optional<std::shared_ptr<Statement>> getStatementById(const std::string& id) const override {
        for (const auto& statement : statements) {
            if (statement && statement->id == id) return statement;
        }
        return std::nullopt;
    }
    void removeStatement(const std::string&) override {}
    void updateStatement(const std::shared_ptr<Statement>&) override {}
    void upsertStatement(const std::shared_ptr<Statement>& statement) override { upsertCalled = true; (void)statement; }
    void clearStatements() override { statements.clear(); }
};

class FakeTransactionRepository : public ITransactionRepository {
public:
    std::vector<std::shared_ptr<Transaction>> transactions;
    bool upsertCalled = false;

    void addTransaction(const std::shared_ptr<Transaction>& transaction) override { transactions.push_back(transaction); }
    std::vector<std::shared_ptr<Transaction>> getTransactions() const override { return transactions; }
    std::optional<std::shared_ptr<Transaction>> getTransactionById(const std::string& id) const override {
        for (const auto& transaction : transactions) {
            if (transaction && transaction->id == id) return transaction;
        }
        return std::nullopt;
    }
    void removeTransaction(const std::string&) override {}
    void updateTransaction(const std::shared_ptr<Transaction>&) override {}
    void upsertTransaction(const std::shared_ptr<Transaction>& transaction) override { upsertCalled = true; (void)transaction; }
    void clearTransactions() override { transactions.clear(); }
    std::vector<std::shared_ptr<Transaction>> getTransactionsForContract(const std::string& contractId) const override {
        std::vector<std::shared_ptr<Transaction>> out;
        for (const auto& transaction : transactions) {
            if (transaction && transaction->contractId == contractId) out.push_back(transaction);
        }
        return out;
    }
    void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) override {
        for (const auto& transaction : transactions) {
            if (!transaction) continue;
            if (std::find(transactionIds.begin(), transactionIds.end(), transaction->id) != transactionIds.end()) {
                transaction->contractId = contractId;
            }
        }
    }
};

std::shared_ptr<Actor> makeActor(const std::string& id, const std::string& name)
{
    auto actor = std::make_shared<Actor>();
    actor->id = id;
    actor->name = name;
    return actor;
}

std::shared_ptr<Property> makeProperty(const std::string& id, const std::string& name)
{
    auto property = std::make_shared<Property>();
    property->id = id;
    property->name = name;
    return property;
}

std::shared_ptr<Contract> makeContract(const std::string& id, const std::string& name)
{
    auto contract = std::make_shared<Contract>();
    contract->id = id;
    contract->name = name;
    return contract;
}

} // namespace

TEST(AppStateManagerTests, SaveAndLoadExportLogs)
{
    AppStateManager::Repositories repos;
    auto exportRepo = std::make_shared<FakeExportLogRepository>();
    repos.exportLogs = exportRepo;

    AppStateManager mgr(repos);

    AppState state;
    auto log = std::make_shared<ExportLog>();
    log->id = "exp-1";
    log->time = "2026-01-01T12:00:00";
    log->targetPath = "C:/tmp/export.zip";
    log->status = "Success";
    log->message = "Done";
    log->payload = "{}";
    state.exportLogs.push_back(log);

    mgr.save(state);
    EXPECT_TRUE(exportRepo->upsertCalled);

    AppState loaded = mgr.load();
    ASSERT_EQ(loaded.exportLogs.size(), 1u);
    ASSERT_TRUE(loaded.exportLogs.front());
    EXPECT_EQ(loaded.exportLogs.front()->id, "exp-1");
    EXPECT_EQ(loaded.exportLogs.front()->status, "Success");
}

TEST(AppStateManagerTests, LoadRehydratesRelationships)
{
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<FakeActorRepository>();
    auto propRepo = std::make_shared<FakePropertyRepository>();
    auto contractRepo = std::make_shared<FakeContractRepository>();
    auto stmtRepo = std::make_shared<FakeStatementRepository>();
    auto txRepo = std::make_shared<FakeTransactionRepository>();

    actorRepo->actors.push_back(makeActor("A1", "Ann"));
    propRepo->properties.push_back(makeProperty("P1", "House"));

    auto contract = makeContract("C1", "Cname");
    contract->actorIds = {"A1", "A1"};
    contract->propertyIds = {"P1", "P1"};
    contractRepo->contracts.push_back(contract);

    repos.actors = actorRepo;
    repos.properties = propRepo;
    repos.contracts = contractRepo;
    repos.statements = stmtRepo;
    repos.transactions = txRepo;

    AppStateManager mgr(repos);
    AppState s = mgr.load();

    ASSERT_EQ(s.actors.size(), 1u);
    ASSERT_EQ(s.properties.size(), 1u);
    ASSERT_EQ(s.contracts.size(), 1u);

    const auto cptr = s.contracts.front();
    ASSERT_TRUE(cptr);
    EXPECT_EQ(cptr->actorIds, (std::vector<std::string>{"A1"}));
    EXPECT_EQ(cptr->propertyIds, (std::vector<std::string>{"P1"}));
}

TEST(AppStateManagerTests, SaveSyncsIdsAndCallsRepos)
{
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<FakeActorRepository>();
    auto propRepo = std::make_shared<FakePropertyRepository>();
    auto contractRepo = std::make_shared<FakeContractRepository>();
    auto stmtRepo = std::make_shared<FakeStatementRepository>();
    auto txRepo = std::make_shared<FakeTransactionRepository>();

    repos.actors = actorRepo;
    repos.properties = propRepo;
    repos.contracts = contractRepo;
    repos.statements = stmtRepo;
    repos.transactions = txRepo;

    AppStateManager mgr(repos);

    AppState state;
    auto actor = makeActor("", "Zoe");
    state.actors.push_back(actor);

    auto property = makeProperty("", "Name");
    state.properties.push_back(property);

    auto contract = makeContract("", "Ct");
    contract->actorIds.push_back(actor->id);
    contract->propertyIds.push_back(property->id);
    state.contracts.push_back(contract);

    mgr.save(state);

    EXPECT_TRUE(actorRepo->upsertCalled);
    EXPECT_TRUE(propRepo->upsertCalled);
    EXPECT_TRUE(contractRepo->upsertCalled);
}

TEST(AppStateManagerTests, ValidateThrowsOnInvalidContractWhenStrict)
{
    AppStateManager::Repositories repos;
    auto contractRepo = std::make_shared<FakeContractRepository>();
    repos.contracts = contractRepo;

    AppStateManager mgr(repos);
    mgr.setStrictValidation(true);

    AppState state;
    auto contract = makeContract("C2", "");
    state.contracts.push_back(contract);

    EXPECT_THROW(mgr.save(state), std::runtime_error);
}

TEST(AppStateManagerTests, LoadThrowsOnUnresolvedTransactionReferenceWhenStrict)
{
    AppStateManager::Repositories repos;
    auto txRepo = std::make_shared<FakeTransactionRepository>();
    repos.transactions = txRepo;

    auto tx = std::make_shared<Transaction>();
    tx->id = "T1";
    tx->name = "Rent";
    tx->actorId = "missing-actor";
    txRepo->transactions.push_back(tx);

    AppStateManager mgr(repos);
    mgr.setStrictValidation(true);

    EXPECT_THROW(mgr.load(), std::runtime_error);
}

TEST(AppStateManagerTests, SaveProjectsContractRelationsWithoutMutatingOriginalState)
{
    AppStateManager::Repositories repos;
    auto contractRepo = std::make_shared<FakeContractRepository>();
    repos.contracts = contractRepo;

    AppStateManager mgr(repos);

    AppState state;
    auto contract = makeContract("C3", "Lease");
    contract->actorIds = {"A2", "A1", "A1"};
    contract->propertyIds = {"P2", "P1", "P1"};
    state.contracts.push_back(contract);

    mgr.save(state);

    ASSERT_TRUE(contractRepo->lastUpserted);
    EXPECT_EQ(contractRepo->lastUpserted->actorIds, (std::vector<std::string>{"A1", "A2"}));
    EXPECT_EQ(contractRepo->lastUpserted->propertyIds, (std::vector<std::string>{"P1", "P2"}));
    EXPECT_EQ(state.contracts.front()->actorIds, (std::vector<std::string>{"A2", "A1", "A1"}));
    EXPECT_EQ(state.contracts.front()->propertyIds, (std::vector<std::string>{"P2", "P1", "P1"}));
}
