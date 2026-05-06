/**
 * @file core/tests/unit/TestApplicationServices.cpp
 * @brief Unit tests for refactored `core/application` services.
 */

#include "gtest/gtest.h"

#include "core/application/CatalogService.h"
#include "core/models/Actor.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include <memory>
#include <vector>

using core::application::CatalogService;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Statement;
using core::domain::Transaction;

TEST(CatalogServiceTests, ActorLifecycleMutatesState)
{
    AppState state;
    CatalogService service;

    const std::string actorId = service.addActor(state, {.name = "Alice"});

    ASSERT_EQ(state.actors.size(), 1u);
    EXPECT_EQ(state.actors.front()->id, actorId);
    EXPECT_EQ(state.actors.front()->name, "Alice");

    EXPECT_TRUE(service.updateActor(state, actorId, {.name = "Bob"}));
    EXPECT_EQ(state.actors.front()->name, "Bob");

    EXPECT_TRUE(service.deleteActor(state, actorId));
    EXPECT_TRUE(state.actors.empty());
}

TEST(CatalogServiceTests, DeleteStatementRemovesDependentTransactions)
{
    AppState state;

    auto statement = std::make_shared<Statement>();
    statement->id = "stmt-1";
    statement->name = "January";
    state.statements.push_back(statement);

    auto matchingTransaction = std::make_shared<Transaction>();
    matchingTransaction->id = "tx-1";
    matchingTransaction->statementId = statement->id;
    state.transactions.push_back(matchingTransaction);

    auto otherTransaction = std::make_shared<Transaction>();
    otherTransaction->id = "tx-2";
    otherTransaction->statementId = "stmt-2";
    state.transactions.push_back(otherTransaction);

    CatalogService service;

    EXPECT_TRUE(service.deleteStatement(state, statement->id));
    EXPECT_TRUE(state.statements.empty());
    ASSERT_EQ(state.transactions.size(), 1u);
    EXPECT_EQ(state.transactions.front()->id, "tx-2");
}

TEST(CatalogServiceTests, AddTransactionRejectsBlankStatementId)
{
    AppState state;
    CatalogService service;

    core::application::TransactionInput input;
    input.name = "Rent";
    input.statementId = "   ";

    EXPECT_TRUE(service.addTransaction(state, input).empty());
    EXPECT_TRUE(state.transactions.empty());
}

TEST(CatalogServiceTests, ContractTypesReturnsSortedDistinctValues)
{
    AppState state;

    auto first = std::make_shared<Contract>();
    first->type = " B ";
    state.contracts.push_back(first);

    auto second = std::make_shared<Contract>();
    second->type = "A";
    state.contracts.push_back(second);

    auto duplicate = std::make_shared<Contract>();
    duplicate->type = "B";
    state.contracts.push_back(duplicate);

    auto blank = std::make_shared<Contract>();
    blank->type = "   ";
    state.contracts.push_back(blank);

    CatalogService service;

    EXPECT_EQ(service.contractTypes(state), (std::vector<std::string>{"A", "B"}));
}

