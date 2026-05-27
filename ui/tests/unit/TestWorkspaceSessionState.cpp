/**
 * @file ui/tests/unit/TestWorkspaceSessionState.cpp
 * @brief Tests for the UI SessionState boundary and derived filters.
 */

#include <gtest/gtest.h>

#include "core/application/storage/DeletionImpact.h"
#include "ui/state/session/WorkspaceSessionState.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/viewmodels/booking/TransactionListModel.h"

namespace ui {

TEST(WorkspaceSessionStateTest, ExposesFilteredTransactionViewsForWorkspaceCollections)
{
    SessionState store;
    store.loadFromState(tests::support::makeWorkspaceCatalog());

    TransactionFilter* statementFilter = store.statementTransactions(QStringLiteral("statement-1"));
    ASSERT_NE(statementFilter, nullptr);
    EXPECT_EQ(statementFilter->rowCount(), 2);

    TransactionFilter* propertyFilter = store.propertyTransactions(QStringLiteral("property-1"));
    ASSERT_NE(propertyFilter, nullptr);
    EXPECT_EQ(propertyFilter->rowCount(), 2);

    propertyFilter->setTxType(QStringLiteral("lease"));
    EXPECT_EQ(propertyFilter->rowCount(), 1);
}

TEST(WorkspaceSessionStateTest, AppliesDeletionImpactAcrossModelsAndFilters)
{
    SessionState store;
    store.loadFromState(tests::support::makeWorkspaceCatalog());

    TransactionFilter* statementFilter = store.statementTransactions(QStringLiteral("statement-1"));
    TransactionFilter* propertyFilter = store.propertyTransactions(QStringLiteral("property-1"));
    ASSERT_NE(statementFilter, nullptr);
    ASSERT_NE(propertyFilter, nullptr);
    EXPECT_EQ(statementFilter->rowCount(), 2);
    EXPECT_EQ(propertyFilter->rowCount(), 2);

    core::domain::DeletionImpact impact;
    impact.deletedStatementIds = {std::string("statement-1")};
    impact.deletedTransactionIds = {std::string("tx-1"), std::string("tx-2")};
    impact.deletedPropertyIds = {std::string("property-1")};

    store.applyDeletionImpact(impact);

    EXPECT_EQ(store.models().statements().rowCount(), 0);
    EXPECT_EQ(store.models().transactions().rowCount(), 0);
    EXPECT_EQ(store.models().properties().rowCount(), 0);

    TransactionFilter* refreshedStatementFilter = store.statementTransactions(QStringLiteral("statement-1"));
    TransactionFilter* refreshedPropertyFilter = store.propertyTransactions(QStringLiteral("property-1"));
    ASSERT_NE(refreshedStatementFilter, nullptr);
    ASSERT_NE(refreshedPropertyFilter, nullptr);
    EXPECT_EQ(refreshedStatementFilter->rowCount(), 0);
    EXPECT_EQ(refreshedPropertyFilter->rowCount(), 0);
}

TEST(WorkspaceSessionStateTest, UpdatesTransactionPropertyAssignmentsInPlace)
{
    SessionState store;
    store.loadFromState(tests::support::makeWorkspaceCatalog());

    TransactionFilter* propertyFilter = store.propertyTransactions(QStringLiteral("property-1"));
    ASSERT_NE(propertyFilter, nullptr);
    EXPECT_EQ(propertyFilter->rowCount(), 2);

    store.setTransactionPropertyIdsImmediate(QStringLiteral("tx-2"), QStringList{QStringLiteral("property-2")});

    const QModelIndex rowIndex = store.models().transactions().index(1, 0);
    ASSERT_TRUE(rowIndex.isValid());
    EXPECT_EQ(store.models().transactions().data(rowIndex, TransactionList::PropertyIdsRole).toList(),
              QVariantList({QStringLiteral("property-2")}));

    EXPECT_EQ(propertyFilter->rowCount(), 1);
}

TEST(WorkspaceSessionStateTest, ResolvesAmountForCommitWithParserAndPersistedFallback)
{
    SessionState store;
    store.loadFromState(tests::support::makeWorkspaceCatalog());

    EXPECT_DOUBLE_EQ(store.amountForTransactionCommit(QStringLiteral("12,75"),
                                                      QStringLiteral(""),
                                                      0.0),
                     12.75);
    EXPECT_DOUBLE_EQ(store.amountForTransactionCommit(QStringLiteral("not-a-number"),
                                                      QStringLiteral("tx-1"),
                                                      0.0),
                     1250.0);
    EXPECT_DOUBLE_EQ(store.amountForTransactionCommit(QVariant(),
                                                      QStringLiteral("missing"),
                                                      7.5),
                    7.5);
}

TEST(WorkspaceSessionStateTest, ComparesActorStateByNormalizedCollections)
{
    SessionState store;

    EXPECT_FALSE(store.formStateChanged(QStringLiteral("Alice"),
                                        QVariantList{QStringLiteral("One"), QStringLiteral("Two")},
                                        QVariantList{QStringLiteral("contract-1")},
                                        QStringLiteral("Alice"),
                                        QVariantList{QStringLiteral("Two"), QStringLiteral("One")},
                                        QVariantList{QStringLiteral("contract-1")}));

    EXPECT_TRUE(store.formStateChanged(QStringLiteral("Alice"),
                                       QVariantList{QStringLiteral("One")},
                                       QVariantList{QStringLiteral("contract-1")},
                                       QStringLiteral("Alice"),
                                       QVariantList{QStringLiteral("One"), QStringLiteral("Two")},
                                       QVariantList{QStringLiteral("contract-1")}));
}

} // namespace ui
