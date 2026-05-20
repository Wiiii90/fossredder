/**
 * @file core/tests/application/workspace/TestWorkspaceCommandService.cpp
 * @brief Tests for workspace command routing and catalog commits.
 */

#include <gtest/gtest.h>

#include <algorithm>

#include "core/application/workspace/WorkspaceCommandService.h"
#include "core/application/workspace/WorkspaceSession.h"
#include "core/domain/entities/Statement.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceCommandServiceTest, RoutesCatalogCommandsThroughCommitBoundary) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::StatementCommand addCommand;
    addCommand.name = "Statement";
    const auto statementId = service.addStatement(addCommand);

    EXPECT_FALSE(statementId.empty());
    EXPECT_EQ(storagePtr->savedState_.catalog.statements().size(), 1u);
    ASSERT_FALSE(storagePtr->savedState_.catalog.statements().empty());
    EXPECT_EQ(storagePtr->savedState_.catalog.statements().front()->name(), "Statement");

    core::ports::workspace::StatementCommand updateCommand;
    updateCommand.id = statementId;
    updateCommand.name = "Statement Updated";
    service.updateStatement(updateCommand);

    ASSERT_FALSE(storagePtr->savedState_.catalog.statements().empty());
    EXPECT_EQ(storagePtr->savedState_.catalog.statements().front()->name(), "Statement Updated");

    service.deleteStatement(statementId);

    EXPECT_TRUE(storagePtr->savedState_.catalog.statements().empty());
}

TEST(WorkspaceCommandServiceTest, InsertsTransactionAfterRequestedStatementTransactionAndKeepsValuta) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    auto* storagePtr = storage.get();
    WorkspaceSession session(std::move(storage));
    session.newFile("P:/workspace.db");

    WorkspaceCommandService service(session);

    core::ports::workspace::StatementCommand statementCommand;
    statementCommand.name = "Statement";
    const auto statementId = service.addStatement(statementCommand);
    ASSERT_FALSE(statementId.empty());

    core::ports::workspace::TransactionCommand first;
    first.name = "First";
    first.bookingDate = "2026-01-01";
    first.valuta = "2026-01-03";
    first.amount = 10.0;
    first.statementId = statementId;
    const auto firstId = service.addTransaction(first);
    ASSERT_FALSE(firstId.empty());

    core::ports::workspace::TransactionCommand second = first;
    second.name = "Second";
    second.bookingDate = "2026-01-02";
    second.valuta = "2026-01-04";
    const auto secondId = service.addTransaction(second);
    ASSERT_FALSE(secondId.empty());

    core::ports::workspace::TransactionCommand inserted = first;
    inserted.name = "Inserted";
    inserted.bookingDate = "2026-01-05";
    inserted.valuta = "2026-01-06";
    inserted.insertAfterTransactionId = firstId;
    const auto insertedId = service.addTransaction(inserted);
    ASSERT_FALSE(insertedId.empty());

    ASSERT_EQ(storagePtr->savedState_.catalog.statements().size(), 1u);
    const auto& ids = storagePtr->savedState_.catalog.statements().front()->transactionIds();
    ASSERT_EQ(ids.size(), 3u);
    EXPECT_EQ(ids[0], firstId);
    EXPECT_EQ(ids[1], insertedId);
    EXPECT_EQ(ids[2], secondId);

    const auto transactions = storagePtr->savedState_.catalog.transactions();
    const auto match = std::find_if(transactions.begin(), transactions.end(), [&](const auto& tx) {
        return tx && tx->id() == insertedId;
    });
    ASSERT_NE(match, transactions.end());
    EXPECT_EQ((*match)->valuta(), "2026-01-06");
}

} // namespace core::application
