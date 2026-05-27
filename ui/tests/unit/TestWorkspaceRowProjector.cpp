/**
 * @file ui/tests/unit/TestWorkspaceRowProjector.cpp
 * @brief Tests for workspace row and selection projection helpers.
 */

#include <gtest/gtest.h>

#include "ui/adapters/core/WorkspaceRowProjector.h"
#include "ui/state/session/WorkspaceSessionState.h"
#include "support/WorkspaceTestData.h"

namespace ui {

TEST(WorkspaceRowProjectorTest, ProjectsStableRowsForAllWorkspaceFamilies)
{
    SessionState store;
    store.loadFromState(tests::support::makeWorkspaceCatalog());

    const QVariantList actorRows = buildActorRows(store);
    const QVariantList propertyRows = buildPropertyRows(store);
    const QVariantList contractRows = buildContractRows(store);
    const QVariantList analysisRows = buildAnalysisRows(store);
    const QVariantList annualRows = buildAnnualRows(store);
    const QVariantList statementRows = buildStatementRows(store);
    const QVariantList transactionRows = buildStatementTransactionRows(store, QStringLiteral("statement-1"));

    ASSERT_EQ(actorRows.size(), 1);
    ASSERT_EQ(propertyRows.size(), 1);
    ASSERT_EQ(contractRows.size(), 1);
    ASSERT_EQ(analysisRows.size(), 1);
    ASSERT_EQ(annualRows.size(), 1);
    ASSERT_EQ(statementRows.size(), 1);
    ASSERT_EQ(transactionRows.size(), 2);

    EXPECT_EQ(actorRows.first().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("actor-1"));
    EXPECT_EQ(actorRows.first().toMap().value(QStringLiteral("aliases")).toList().first().toString(), QStringLiteral("Primary Actor"));
    EXPECT_EQ(actorRows.first().toMap().value(QStringLiteral("selectedIds")).toList().first().toString(), QStringLiteral("contract-1"));
    EXPECT_EQ(propertyRows.first().toMap().value(QStringLiteral("name")).toString(), QStringLiteral("Primary Property"));
    EXPECT_EQ(propertyRows.first().toMap().value(QStringLiteral("aliases")).toList().first().toString(), QStringLiteral("Property Alias"));
    EXPECT_EQ(propertyRows.first().toMap().value(QStringLiteral("selectedIds")).toList().first().toString(), QStringLiteral("contract-1"));
    EXPECT_EQ(contractRows.first().toMap().value(QStringLiteral("type")).toString(), QStringLiteral("lease"));
    EXPECT_EQ(analysisRows.first().toMap().value(QStringLiteral("adjustments")).toString(), QStringLiteral("{\"actor-1\":19.25}"));
    EXPECT_EQ(annualRows.first().toMap().value(QStringLiteral("year")).toInt(), 2026);
    EXPECT_EQ(statementRows.first().toMap().value(QStringLiteral("name")).toString(), QStringLiteral("January Statement"));
    EXPECT_EQ(transactionRows.first().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("tx-1"));
}

TEST(WorkspaceRowProjectorTest, PreservesGenericOrderingAndSelectionContracts)
{
    const QVariantList rows{
        QVariantMap{{QStringLiteral("id"), QStringLiteral("a")}, {QStringLiteral("display"), QStringLiteral("Alpha")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("b")}, {QStringLiteral("display"), QStringLiteral("Beta")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("c")}, {QStringLiteral("display"), QStringLiteral("Gamma")}}
    };

    EXPECT_EQ(pruneAndAppendMissing(QVariantList{QStringLiteral("b"), QStringLiteral("x"), QStringLiteral("a")},
                                    QVariantList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}),
              QVariantList({QStringLiteral("b"), QStringLiteral("a"), QStringLiteral("c")}));

    EXPECT_EQ(rowIds(rows), QVariantList({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}));
    EXPECT_EQ(orderedRowsByIds(rows, QVariantList{QStringLiteral("c"), QStringLiteral("a")}).at(0).toMap().value(QStringLiteral("id")).toString(),
              QStringLiteral("c"));
    EXPECT_EQ(indexOfId(rows, QStringLiteral("b")), 1);
    EXPECT_EQ(indexOfKeyValue(rows, QStringLiteral("display"), QStringLiteral("Gamma")), 2);
    EXPECT_EQ(indexOfString(QVariantList{QStringLiteral("a"), QStringLiteral("b")}, QStringLiteral("b")), 1);
    EXPECT_EQ(normalizedIndex(-3, 2), 0);
    EXPECT_EQ(wrappedIndex(-1, 3), 2);
    EXPECT_EQ(wrappedIdAt(rows, 4), QStringLiteral("b"));
    EXPECT_EQ(navigatedId(rows, QStringLiteral("b"), 1), QStringLiteral("c"));
    EXPECT_EQ(navigatedSelectionId(rows, QStringLiteral("b"), 1, 0), QStringLiteral("c"));
    EXPECT_EQ(navigatedSelectionId(rows, QStringLiteral("c"), 1, 0), QString());
    EXPECT_EQ(navigatedSelectionId(rows, QString(), 1, 0), QStringLiteral("a"));
    EXPECT_EQ(displayRowsWithEmpty(rows, QStringLiteral("None")).first().toMap().value(QStringLiteral("display")).toString(), QStringLiteral("None"));

    const QVariantMap resolved = resolveSelectionState(rows, 1, QStringLiteral("b"));
    EXPECT_EQ(resolved.value(QStringLiteral("id")).toString(), QStringLiteral("b"));
    EXPECT_EQ(resolved.value(QStringLiteral("currentId")).toString(), QStringLiteral("b"));

    const QVariantList inserted = orderWithInsertedId(QVariantList{QStringLiteral("a"), QStringLiteral("c")},
                                                      QVariantList{QStringLiteral("a"), QStringLiteral("c")},
                                                      QStringLiteral("b"),
                                                      0);
    ASSERT_EQ(inserted.size(), 3);
    EXPECT_EQ(inserted.at(0).toString(), QStringLiteral("a"));
    EXPECT_EQ(inserted.at(1).toString(), QStringLiteral("b"));
    EXPECT_EQ(inserted.at(2).toString(), QStringLiteral("c"));

    const QVariantMap orderedRows = orderedRowsState(rows, QVariantList{QStringLiteral("c"), QStringLiteral("a")});
    EXPECT_EQ(orderedRows.value(QStringLiteral("rows")).toList().first().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("c"));
    EXPECT_EQ(orderedRows.value(QStringLiteral("order")).toList(), QVariantList({QStringLiteral("c"), QStringLiteral("a"), QStringLiteral("b")}));
    EXPECT_EQ(orderedRows.value(QStringLiteral("orderIds")).toList(), QVariantList({QStringLiteral("c"), QStringLiteral("a"), QStringLiteral("b")}));

    const QVariantMap orderedSelection = orderedSelectionState(rows, QVariantList{QStringLiteral("c"), QStringLiteral("a")}, 1, QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("selection")).toMap().value(QStringLiteral("id")).toString(), QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("id")).toString(), QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("currentId")).toString(), QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("index")).toInt(), 2);
    EXPECT_EQ(orderedSelection.value(QStringLiteral("orderIds")).toList(), QVariantList({QStringLiteral("c"), QStringLiteral("a"), QStringLiteral("b")}));

    const QVariantMap selectedById = orderedSelectionState(rows, QVariantList{QStringLiteral("c"), QStringLiteral("a")}, 0, QStringLiteral("b"));
    EXPECT_EQ(selectedById.value(QStringLiteral("id")).toString(), QStringLiteral("b"));
    EXPECT_EQ(selectedById.value(QStringLiteral("currentId")).toString(), QStringLiteral("b"));
    EXPECT_EQ(selectedById.value(QStringLiteral("index")).toInt(), 2);

    const QVariantMap navigation = navigateSelectionState(rows, 1, QStringLiteral("b"), 1);
    EXPECT_EQ(navigation.value(QStringLiteral("id")).toString(), QStringLiteral("c"));

    const QVariantMap reselection = deleteReselectionState(rows, QVariantList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}, 1, QStringLiteral("b"));
    EXPECT_EQ(reselection.value(QStringLiteral("nextId")).toString(), QStringLiteral("c"));

    EXPECT_EQ(deleteNextSelectionId(rows, QStringLiteral("b")), QStringLiteral("c"));

    const QVariantMap formState = basicFormState(QStringLiteral("Demo"),
                                                QVariantList{QStringLiteral(" alpha "), QStringLiteral("beta")},
                                                QVariantList{QStringLiteral("actor-1")});
    EXPECT_EQ(formState.value(QStringLiteral("name")).toString(), QStringLiteral("Demo"));
    EXPECT_EQ(formState.value(QStringLiteral("aliases")).toList().size(), 2);
    EXPECT_EQ(formState.value(QStringLiteral("selectedIds")).toList().first().toString(), QStringLiteral("actor-1"));

    const QVariantMap contractState = contractFormState(QStringLiteral("Contract"),
                                                       QStringLiteral("lease"),
                                                       QVariantList{QStringLiteral("actor-1"), QStringLiteral("actor-2")},
                                                       QVariantList{QStringLiteral("property-1")},
                                                       QVariantList{QStringLiteral("alias-1")});
    EXPECT_EQ(contractState.value(QStringLiteral("type")).toString(), QStringLiteral("lease"));
    EXPECT_EQ(contractState.value(QStringLiteral("selectedActorIds")).toList().size(), 1);
    EXPECT_EQ(contractState.value(QStringLiteral("selectedPropertyIds")).toList().first().toString(), QStringLiteral("property-1"));
}

} // namespace ui
