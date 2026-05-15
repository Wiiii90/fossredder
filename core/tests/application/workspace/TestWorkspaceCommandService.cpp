/**
 * @file core/tests/application/workspace/TestWorkspaceCommandService.cpp
 * @brief Tests for workspace command routing and catalog commits.
 */

#include <gtest/gtest.h>

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

} // namespace core::application
