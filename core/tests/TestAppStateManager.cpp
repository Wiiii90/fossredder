/**
 * @file core/tests/TestAppStateManager.cpp
 * @brief Unit tests for AppStateManager (UNIT).
 */

#include "gtest/gtest.h"

#include "core/managers/AppStateManager.h"
#include "core/repositories/IActorRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"

#include "core/models/Actor.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

// Minimal fake repository implementations for testing
class FakeActorRepository : public IActorRepository {
public:
    std::vector<std::shared_ptr<Actor>> actors;
    bool upsertCalled = false;

    void addActor(const std::shared_ptr<Actor>& actor) override { actors.push_back(actor); }
    std::vector<std::shared_ptr<Actor>> getActors() const override { return actors; }
    std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const override {
        for (auto& a : actors) if (a && a->id == id) return a;
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
        for (auto& p : properties) if (p && p->id == id) return p;
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
    void addContract(const std::shared_ptr<Contract>& contract) override { contracts.push_back(contract); }
    std::vector<std::shared_ptr<Contract>> getContracts() const override { return contracts; }
    std::optional<std::shared_ptr<Contract>> getContractById(const std::string& id) const override {
        for (auto& c : contracts) if (c && c->id == id) return c;
        return std::nullopt;
    }
    void removeContract(const std::string&) override {}
    void updateContract(const std::shared_ptr<Contract>&) override {}
    void upsertContract(const std::shared_ptr<Contract>& contract) override { upsertCalled = true; (void)contract; }
    void clearContracts() override { contracts.clear(); }
};

class FakeStatementRepository : public IStatementRepository {
public:
    std::vector<std::shared_ptr<Statement>> statements;
    bool upsertCalled = false;
    void addStatement(const std::shared_ptr<Statement>& statement) override { statements.push_back(statement); }
    std::vector<std::shared_ptr<Statement>> getStatements() const override { return statements; }
    std::optional<std::shared_ptr<Statement>> getStatementById(const std::string& id) const override {
        for (auto& s : statements) if (s && s->id == id) return s;
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
        for (auto& t : transactions) if (t && t->id == id) return t;
        return std::nullopt;
    }
    void removeTransaction(const std::string&) override {}
    void updateTransaction(const std::shared_ptr<Transaction>&) override {}
    void upsertTransaction(const std::shared_ptr<Transaction>& transaction) override { upsertCalled = true; (void)transaction; }
    void clearTransactions() override { transactions.clear(); }
};

TEST(AppStateManagerTests, LoadRehydratesRelationships) {
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<FakeActorRepository>();
    auto propRepo = std::make_shared<FakePropertyRepository>();
    auto contractRepo = std::make_shared<FakeContractRepository>();
    auto stmtRepo = std::make_shared<FakeStatementRepository>();
    auto txRepo = std::make_shared<FakeTransactionRepository>();

    // Prepare underlying data: actor and property
    auto actor = std::make_shared<Actor>(); actor->id = "A1"; actor->name = "Ann";
    actorRepo->actors.push_back(actor);
    auto prop = std::make_shared<Property>("House","Addr","Desc"); prop->id = "P1";
    propRepo->properties.push_back(prop);

    // contract references by id
    auto contract = std::make_shared<Contract>(); contract->id = "C1"; contract->name = "Cname";
    contract->actorIds.push_back("A1"); contract->propertyIds.push_back("P1");
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

    // After rehydrate, contract should have pointers to actor and property
    auto cptr = s.contracts.front();
    ASSERT_TRUE(cptr);
    EXPECT_FALSE(cptr->actorIds.empty());
    EXPECT_FALSE(cptr->propertyIds.empty());
    EXPECT_EQ(cptr->actors.size(), 1u);
    EXPECT_EQ(cptr->properties.size(), 1u);
    EXPECT_EQ(cptr->actors.front()->id, std::string("A1"));
    EXPECT_EQ(cptr->properties.front()->id, std::string("P1"));
}

TEST(AppStateManagerTests, SaveSyncsIdsAndCallsRepos) {
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
    auto a = std::make_shared<Actor>(); a->id = ""; a->name = "Zoe"; // no id -> will be upserted
    state.actors.push_back(a);

    auto prop = std::make_shared<Property>("Name","Addr","Desc"); prop->id = "";
    state.properties.push_back(prop);

    auto contract = std::make_shared<Contract>(); contract->id = ""; contract->name = "Ct";
    contract->actors.push_back(a.get());
    contract->properties.push_back(prop.get());
    state.contracts.push_back(contract);

    mgr.save(state);

    // verify repos were asked to upsert
    EXPECT_TRUE(actorRepo->upsertCalled);
    EXPECT_TRUE(propRepo->upsertCalled);
    EXPECT_TRUE(contractRepo->upsertCalled);
}

TEST(AppStateManagerTests, ValidateThrowsOnInvalidContractWhenStrict) {
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
    mgr.setStrictValidation(true);

    AppState state;
    auto c = std::make_shared<Contract>(); c->id = "C2"; c->name = ""; // empty name should trigger
    state.contracts.push_back(c);

    EXPECT_THROW(mgr.save(state), std::runtime_error);
}
