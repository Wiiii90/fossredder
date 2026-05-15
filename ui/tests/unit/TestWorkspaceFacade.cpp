/**
 * @file ui/tests/unit/TestWorkspaceFacade.cpp
 * @brief Tests for the UI WorkspaceFacade boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/workspace/WorkspaceFacade.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

TEST(WorkspaceFacadeTest, LoadsTheWorkspaceProjectionAndSelectionState)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    facade.loadFromState(tests::support::makeWorkspaceCatalog());

    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.properties()->rowCount(), 1);
    EXPECT_EQ(facade.contracts()->rowCount(), 1);
    EXPECT_EQ(facade.statements()->rowCount(), 1);
    EXPECT_EQ(facade.transactions()->rowCount(), 2);
    EXPECT_EQ(facade.analyses()->rowCount(), 1);
    EXPECT_EQ(facade.annuals()->rowCount(), 1);

    facade.setSelectedActorId(QStringLiteral("actor-1"));
    facade.setSelectedPropertyId(QStringLiteral("property-1"));
    facade.setSelectedContractId(QStringLiteral("contract-1"));
    facade.setSelectedStatementId(QStringLiteral("statement-1"));
    facade.setSelectedTransactionId(QStringLiteral("tx-1"));
    facade.setSelectedAnalysisId(QStringLiteral("analysis-1"));
    facade.setSelectedAnnualId(QStringLiteral("annual-1"));

    ASSERT_NE(facade.selectedActor(), nullptr);
    ASSERT_NE(facade.selectedTransaction(), nullptr);
    ASSERT_NE(facade.selectedAnalysis(), nullptr);
    EXPECT_EQ(facade.selectedActor()->id(), QStringLiteral("actor-1"));
    EXPECT_EQ(facade.selectedActor()->name(), QStringLiteral("Main Actor"));
    EXPECT_EQ(facade.selectedTransaction()->id(), QStringLiteral("tx-1"));
    EXPECT_EQ(facade.selectedTransaction()->statementId(), QStringLiteral("statement-1"));
    EXPECT_EQ(facade.selectedAnalysis()->id(), QStringLiteral("analysis-1"));

    EXPECT_EQ(facade.statementTransactionIds(QStringLiteral("statement-1")),
              QVariantList({QStringLiteral("tx-1"), QStringLiteral("tx-2")}));

    EXPECT_EQ(storagePtr->currentPath(), std::string());
    facade.newFile(QStringLiteral("workspace-a.fr"));
    EXPECT_EQ(facade.currentPath(), QStringLiteral("workspace-a.fr"));
    EXPECT_EQ(storagePtr->currentPath(), std::string("workspace-a.fr"));
}

TEST(WorkspaceFacadeTest, RoutesMutationsThroughTheCoreBoundaryAndRefreshesUIRows)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    EXPECT_EQ(facade.actors()->rowCount(), 0);

    const QString createdId = facade.addActor(QStringLiteral("Second Actor"),
                                              QStringList{QStringLiteral("Actor Two")});
    ASSERT_FALSE(createdId.isEmpty());
    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.actorRows().size(), 1);
    EXPECT_EQ(facade.actors()->data(facade.actors()->index(0, 0), ActorList::NameRole).toString(),
              QStringLiteral("Second Actor"));

    const QString thirdId = facade.addActor(QStringLiteral("Third Actor"));
    ASSERT_FALSE(thirdId.isEmpty());
    EXPECT_EQ(facade.actors()->rowCount(), 2);

    facade.deleteActor(createdId);
    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.actors()->data(facade.actors()->index(0, 0), ActorList::NameRole).toString(),
              QStringLiteral("Third Actor"));
}

} // namespace ui
