/**
 * @file core/tests/unit/TestDraftFinalizer.cpp
 * @brief Unit tests for `core::application::DraftFinalizer`.
 */

#include "gtest/gtest.h"

#include "core/application/DraftFinalizer.h"
#include "core/constants/CoreDefaults.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/StatementDraft.h"
#include "core/models/TransactionDraft.h"

#include <memory>
#include <string>
#include <vector>

using core::domain::AppState;
using core::domain::Contract;
using core::domain::StatementDraft;
using core::domain::TransactionDraft;
using core::domain::Transaction;

TEST(DraftFinalizerTests, FinalizeReturnsEmptyWhenDraftHasNoTransactions)
{
    AppState state;
    StatementDraft draft;
    draft.name = "Ignored";

    const auto statementId = core::application::DraftFinalizer::finalize(state, draft);

    EXPECT_TRUE(statementId.empty());
    EXPECT_TRUE(state.empty());
}

TEST(DraftFinalizerTests, FinalizeCreatesStatementTransactionsAndGeneratedContracts)
{
    AppState state;

    auto existingContract = std::make_shared<Contract>();
    existingContract->id = "existing-contract";
    existingContract->name = std::string(core::constants::appState::kGeneratedContractPrefix) + "2";
    state.contracts.push_back(existingContract);

    StatementDraft draft;
    draft.name = "  Imported Statement  ";
    TransactionDraft tx1;
    tx1.name = "Rent";
    tx1.bookingDate = "2025-01-10";
    tx1.amount = 100.0;
    tx1.description = "January";
    tx1.actorId = "actor-1";
    tx1.status = Transaction::Status::Verified;
    tx1.type = " Utility ";
    tx1.allocatable = true;
    tx1.propertyIds = {"property-1"};
    draft.transactions.push_back(tx1);

    const auto statementId = core::application::DraftFinalizer::finalize(state, draft);

    ASSERT_FALSE(statementId.empty());
    ASSERT_EQ(state.statements.size(), 1u);
    EXPECT_EQ(state.statements.front()->id, statementId);
    EXPECT_EQ(state.statements.front()->name, "  Imported Statement  ");

    ASSERT_EQ(state.transactions.size(), 1u);
    const auto& transaction = state.transactions.front();
    EXPECT_EQ(transaction->statementId, statementId);
    EXPECT_EQ(transaction->status, Transaction::Status::Verified);
    EXPECT_EQ(transaction->actorId, "actor-1");
    EXPECT_TRUE(transaction->allocatable);
    EXPECT_TRUE(transaction->valuta.empty());

    ASSERT_EQ(state.contracts.size(), 2u);
    const auto& generatedContract = state.contracts.back();
    const std::vector<std::string> expectedPropertyIds{"property-1"};
    EXPECT_EQ(generatedContract->name, std::string(core::constants::appState::kGeneratedContractPrefix) + "3");
    EXPECT_EQ(generatedContract->type, "Utility");
    EXPECT_EQ(generatedContract->propertyIds, expectedPropertyIds);
    EXPECT_EQ(transaction->contractId, generatedContract->id);
}

TEST(DraftFinalizerTests, FinalizeUsesSelectedContractWhenProvided)
{
    AppState state;

    auto selectedContract = std::make_shared<Contract>();
    selectedContract->id = "contract-42";
    selectedContract->name = "Existing Contract";
    selectedContract->type = "Electricity";
    state.contracts.push_back(selectedContract);

    StatementDraft draft;
    draft.name = "Imported Statement";
    TransactionDraft tx2;
    tx2.name = "Invoice";
    tx2.bookingDate = "2025-02-01";
    tx2.amount = 20.0;
    tx2.description = "Selected contract";
    tx2.status = Transaction::Status::Verified;
    tx2.contractId = "contract-42";
    tx2.type = "Fallback type";
    tx2.propertyIds = {"property-7"};
    draft.transactions.push_back(tx2);

    const auto statementId = core::application::DraftFinalizer::finalize(state, draft);

    ASSERT_FALSE(statementId.empty());
    ASSERT_EQ(state.transactions.size(), 1u);
    EXPECT_EQ(state.transactions.front()->contractId, "contract-42");
    EXPECT_EQ(state.contracts.size(), 1u);
}
