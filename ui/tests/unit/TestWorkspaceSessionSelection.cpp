/**
 * @file ui/tests/unit/TestWorkspaceSessionSelection.cpp
 * @brief Tests for the UI SessionSelection synchronization layer.
 */

#include <gtest/gtest.h>

#include "ui/state/session/WorkspaceSessionSelection.h"
#include "support/WorkspaceTestData.h"

namespace ui {

TEST(WorkspaceSessionSelectionTest, TracksCurrentSelectionAcrossAllCollections)
{
    SessionModels models;
    models.loadFromState(tests::support::makeWorkspaceCatalog());

    SessionSelection selection(models);

    selection.setSelectedActorId(QStringLiteral("actor-1"));
    selection.setSelectedPropertyId(QStringLiteral("property-1"));
    selection.setSelectedContractId(QStringLiteral("contract-1"));
    selection.setSelectedStatementId(QStringLiteral("statement-1"));
    selection.setSelectedTransactionId(QStringLiteral("tx-1"));
    selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));
    selection.setSelectedAnnualId(QStringLiteral("annual-1"));
    selection.setLastAnalysisResult(QVariantMap{{QStringLiteral("ok"), true}});

    EXPECT_EQ(selection.selectedActorId(), QStringLiteral("actor-1"));
    EXPECT_EQ(selection.selectedPropertyId(), QStringLiteral("property-1"));
    EXPECT_EQ(selection.selectedContractId(), QStringLiteral("contract-1"));
    EXPECT_EQ(selection.selectedStatementId(), QStringLiteral("statement-1"));
    EXPECT_EQ(selection.selectedTransactionId(), QStringLiteral("tx-1"));
    EXPECT_EQ(selection.selectedAnalysisId(), QStringLiteral("analysis-1"));
    EXPECT_EQ(selection.selectedAnnualId(), QStringLiteral("annual-1"));
    ASSERT_TRUE(selection.lastAnalysisResult().isValid());

    ASSERT_NE(selection.selectedActor(), nullptr);
    ASSERT_NE(selection.selectedProperty(), nullptr);
    ASSERT_NE(selection.selectedContract(), nullptr);
    ASSERT_NE(selection.selectedStatement(), nullptr);
    ASSERT_NE(selection.selectedTransaction(), nullptr);
    ASSERT_NE(selection.selectedAnalysis(), nullptr);
    ASSERT_NE(selection.selectedAnnual(), nullptr);

    EXPECT_EQ(selection.selectedActor()->id(), QStringLiteral("actor-1"));
    EXPECT_EQ(selection.selectedProperty()->id(), QStringLiteral("property-1"));
    EXPECT_EQ(selection.selectedContract()->id(), QStringLiteral("contract-1"));
    EXPECT_EQ(selection.selectedStatement()->id(), QStringLiteral("statement-1"));
    EXPECT_EQ(selection.selectedTransaction()->id(), QStringLiteral("tx-1"));
    EXPECT_EQ(selection.selectedAnalysis()->id(), QStringLiteral("analysis-1"));
    EXPECT_EQ(selection.selectedAnnual()->id(), QStringLiteral("annual-1"));
}

TEST(WorkspaceSessionSelectionTest, ClearsStaleSelectionsAndAnalysisResultAfterReload)
{
    SessionModels models;
    models.loadFromState(tests::support::makeWorkspaceCatalog());

    SessionSelection selection(models);
    selection.setSelectedActorId(QStringLiteral("actor-1"));
    selection.setSelectedAnalysisId(QStringLiteral("analysis-1"));
    selection.setLastAnalysisResult(QVariantMap{{QStringLiteral("ok"), true}});

    models.actors().removeAt(0);
    models.analyses().removeAt(0);

    EXPECT_TRUE(selection.selectedActorId().isEmpty());
    EXPECT_TRUE(selection.selectedAnalysisId().isEmpty());
    EXPECT_FALSE(selection.lastAnalysisResult().isValid());
    EXPECT_TRUE(selection.selectedActor()->id().isEmpty());
    EXPECT_TRUE(selection.selectedAnalysis()->id().isEmpty());

    selection.loadFromState();
    EXPECT_TRUE(selection.selectedActorId().isEmpty());
    EXPECT_TRUE(selection.selectedAnalysisId().isEmpty());
}

} // namespace ui
