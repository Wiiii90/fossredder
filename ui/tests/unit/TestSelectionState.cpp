/**
 * @file ui/tests/unit/TestSelectionState.cpp
 * @brief Tests for the UI SelectionState helper.
 */

#include <gtest/gtest.h>

#include "ui/state/selection/SelectionState.h"
#include "ui/state/session/WorkspaceSessionModels.h"
#include "support/WorkspaceTestData.h"

namespace ui {

TEST(SelectionStateTest, ProjectsSelectionObjectsFromTheCurrentModels)
{
    SessionModels models;
    models.loadFromState(tests::support::makeWorkspaceCatalog());

    SelectionState selection(models.actors(),
                             models.properties(),
                             models.contracts(),
                             models.statements(),
                             models.transactions(),
                             models.analyses(),
                             models.annuals());

    EXPECT_TRUE(selection.setSelectedActorId(QStringLiteral("actor-1")));
    EXPECT_TRUE(selection.setSelectedPropertyId(QStringLiteral("property-1")));
    EXPECT_TRUE(selection.setSelectedContractId(QStringLiteral("contract-1")));
    EXPECT_TRUE(selection.setSelectedStatementId(QStringLiteral("statement-1")));
    EXPECT_TRUE(selection.setSelectedTransactionId(QStringLiteral("tx-1")));
    EXPECT_TRUE(selection.setSelectedAnalysisId(QStringLiteral("analysis-1")));
    EXPECT_TRUE(selection.setSelectedAnnualId(QStringLiteral("annual-1")));

    ASSERT_NE(selection.selectedActor(), nullptr);
    ASSERT_NE(selection.selectedProperty(), nullptr);
    ASSERT_NE(selection.selectedContract(), nullptr);
    ASSERT_NE(selection.selectedStatement(), nullptr);
    ASSERT_NE(selection.selectedTransaction(), nullptr);
    ASSERT_NE(selection.selectedAnalysis(), nullptr);
    ASSERT_NE(selection.selectedAnnual(), nullptr);

    EXPECT_EQ(selection.selectedActor()->id(), QStringLiteral("actor-1"));
    EXPECT_EQ(selection.selectedActor()->name(), QStringLiteral("Main Actor"));
    EXPECT_EQ(selection.selectedActor()->aliases(), QStringList({QStringLiteral("Primary Actor"), QStringLiteral("Main Actor")}));

    EXPECT_EQ(selection.selectedProperty()->id(), QStringLiteral("property-1"));
    EXPECT_EQ(selection.selectedProperty()->name(), QStringLiteral("Primary Property"));
    EXPECT_EQ(selection.selectedProperty()->aliases(), QStringList({QStringLiteral("Property Alias")}));

    EXPECT_EQ(selection.selectedContract()->id(), QStringLiteral("contract-1"));
    EXPECT_EQ(selection.selectedContract()->name(), QStringLiteral("Lease Contract"));
    EXPECT_EQ(selection.selectedContract()->type(), QStringLiteral("lease"));
    EXPECT_EQ(selection.selectedContract()->actorIds(), QStringList({QStringLiteral("actor-1")}));
    EXPECT_EQ(selection.selectedContract()->propertyIds(), QStringList({QStringLiteral("property-1")}));
    EXPECT_EQ(selection.selectedContract()->aliases(), QStringList({QStringLiteral("Lease")}));

    EXPECT_EQ(selection.selectedStatement()->id(), QStringLiteral("statement-1"));
    EXPECT_EQ(selection.selectedStatement()->name(), QStringLiteral("January Statement"));

    EXPECT_EQ(selection.selectedTransaction()->id(), QStringLiteral("tx-1"));
    EXPECT_EQ(selection.selectedTransaction()->name(), QStringLiteral("Rent"));
    EXPECT_EQ(selection.selectedTransaction()->bookingDate(), QStringLiteral("2026-01-05"));
    EXPECT_DOUBLE_EQ(selection.selectedTransaction()->amount(), 1250.0);
    EXPECT_EQ(selection.selectedTransaction()->statementId(), QStringLiteral("statement-1"));
    EXPECT_EQ(selection.selectedTransaction()->actorId(), QStringLiteral("actor-1"));
    EXPECT_EQ(selection.selectedTransaction()->propertyIds(), QStringList({QStringLiteral("property-1")}));
    EXPECT_TRUE(selection.selectedTransaction()->allocatable());

    EXPECT_EQ(selection.selectedAnalysis()->id(), QStringLiteral("analysis-1"));
    EXPECT_EQ(selection.selectedAnalysis()->name(), QStringLiteral("Monthly Analysis"));
    EXPECT_EQ(selection.selectedAnalysis()->type(), QStringLiteral("tabular"));

    EXPECT_EQ(selection.selectedAnnual()->id(), QStringLiteral("annual-1"));
    EXPECT_EQ(selection.selectedAnnual()->name(), QStringLiteral("2026"));
}

TEST(SelectionStateTest, ClearsSelectionsWhenSourceRowsDisappear)
{
    SessionModels models;
    models.loadFromState(tests::support::makeWorkspaceCatalog());

    SelectionState selection(models.actors(),
                             models.properties(),
                             models.contracts(),
                             models.statements(),
                             models.transactions(),
                             models.analyses(),
                             models.annuals());

    selection.setSelectedActorId(QStringLiteral("actor-1"));
    selection.setSelectedContractId(QStringLiteral("contract-1"));
    selection.setSelectedTransactionId(QStringLiteral("tx-1"));
    ASSERT_EQ(selection.selectedActor()->id(), QStringLiteral("actor-1"));
    ASSERT_EQ(selection.selectedContract()->id(), QStringLiteral("contract-1"));
    ASSERT_EQ(selection.selectedTransaction()->id(), QStringLiteral("tx-1"));

    models.actors().removeAt(0);
    models.contracts().removeAt(0);
    models.transactions().removeAt(0);
    selection.refreshAll();

    EXPECT_TRUE(selection.selectedActorId().isEmpty());
    EXPECT_TRUE(selection.selectedContractId().isEmpty());
    EXPECT_TRUE(selection.selectedTransactionId().isEmpty());
    EXPECT_TRUE(selection.selectedActor()->id().isEmpty());
    EXPECT_TRUE(selection.selectedContract()->id().isEmpty());
    EXPECT_TRUE(selection.selectedTransaction()->id().isEmpty());
}

} // namespace ui
