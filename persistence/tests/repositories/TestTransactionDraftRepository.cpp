/**
 * @file persistence/tests/repositories/TestTransactionDraftRepository.cpp
 * @brief Tests for the SQLite-backed transaction draft repository.
 */

#include <gtest/gtest.h>

#include "persistence/repositories/SqliteStatementDraftRepository.h"
#include "persistence/repositories/SqliteTransactionDraftRepository.h"
#include "support/PersistenceTestData.h"

namespace persistence::tests {

TEST(TransactionDraftRepositoryTest, AddsUpdatesRemovesAndClearsTransactionDrafts)
{
    TempDatabase dbFile("transaction-draft-repository");
    SqliteStatementDraftRepository statementDrafts(dbFile.string());
    SqliteTransactionDraftRepository repo(dbFile.string());

    auto statementDraft = makeStatementDraft();
    statementDraft.transactionIds.clear();
    statementDrafts.addStatementDraft(std::make_shared<core::application::importing::draft::StatementDraft>(statementDraft));

    auto draft = makeTransactionDraft("draft-tx-1");
    draft.statementDraftId = "statement-draft-1";
    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(draft));

    ASSERT_EQ(repo.getTransactionDrafts().size(), 1);
    ASSERT_TRUE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->name, "Draft Rent");
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->propertyIds,
              std::vector<std::string>({"property-1"}));

    auto updated = makeTransactionDraft("draft-tx-1");
    updated.statementDraftId = "statement-draft-1";
    updated.name = "Updated Draft Rent";
    updated.propertyIds = { "property-1", "property-2" };
    repo.updateTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(updated));

    ASSERT_TRUE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->name, "Updated Draft Rent");
    EXPECT_EQ(repo.getTransactionDraftById("draft-tx-1").value()->propertyIds,
              std::vector<std::string>({"property-1", "property-2"}));

    repo.removeTransactionDraft("draft-tx-1");
    EXPECT_FALSE(repo.getTransactionDraftById("draft-tx-1").has_value());
    EXPECT_TRUE(repo.getTransactionDrafts().empty());

    repo.addTransactionDraft(std::make_shared<core::application::importing::draft::TransactionDraft>(makeTransactionDraft("draft-tx-2")));
    repo.clearTransactionDrafts();
    EXPECT_TRUE(repo.getTransactionDrafts().empty());
}

} // namespace persistence::tests
