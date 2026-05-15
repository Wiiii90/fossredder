/**
 * @file ui/tests/unit/TestSessionModels.cpp
 * @brief Tests for the UI SessionModels projection layer.
 */

#include <gtest/gtest.h>

#include <QVariant>

#include "ui/state/session/WorkspaceSessionModels.h"
#include "support/WorkspaceTestData.h"
#include "ui/viewmodels/booking/StatementListModel.h"
#include "ui/viewmodels/booking/TransactionListModel.h"
#include "ui/viewmodels/catalog/ActorListModel.h"
#include "ui/viewmodels/catalog/ContractListModel.h"
#include "ui/viewmodels/catalog/PropertyListModel.h"
#include "ui/viewmodels/reporting/AnalysisListModel.h"
#include "ui/viewmodels/reporting/AnnualListModel.h"

namespace ui {

TEST(SessionModelsTest, ProjectsAllCollectionsAndRolesFromTheWorkspaceCatalog)
{
    const auto catalog = tests::support::makeWorkspaceCatalog();

    SessionModels models;
    models.loadFromState(catalog);

    EXPECT_EQ(models.actors().rowCount(), 1);
    EXPECT_EQ(models.properties().rowCount(), 1);
    EXPECT_EQ(models.contracts().rowCount(), 1);
    EXPECT_EQ(models.statements().rowCount(), 1);
    EXPECT_EQ(models.transactions().rowCount(), 2);
    EXPECT_EQ(models.analyses().rowCount(), 1);
    EXPECT_EQ(models.annuals().rowCount(), 1);

    const QModelIndex actorIndex = models.actors().index(0, 0);
    EXPECT_EQ(models.actors().data(actorIndex, ActorList::IdRole).toString(), QStringLiteral("actor-1"));
    EXPECT_EQ(models.actors().data(actorIndex, ActorList::NameRole).toString(), QStringLiteral("Main Actor"));

    const QModelIndex propertyIndex = models.properties().index(0, 0);
    EXPECT_EQ(models.properties().data(propertyIndex, PropertyList::IdRole).toString(), QStringLiteral("property-1"));
    EXPECT_EQ(models.properties().data(propertyIndex, PropertyList::NameRole).toString(), QStringLiteral("Primary Property"));

    const QModelIndex contractIndex = models.contracts().index(0, 0);
    EXPECT_EQ(models.contracts().data(contractIndex, ContractList::IdRole).toString(), QStringLiteral("contract-1"));
    EXPECT_EQ(models.contracts().data(contractIndex, ContractList::NameRole).toString(), QStringLiteral("Lease Contract"));
    EXPECT_EQ(models.contracts().data(contractIndex, ContractList::TypeRole).toString(), QStringLiteral("lease"));
    EXPECT_EQ(models.contracts().data(contractIndex, ContractList::ActorIdsRole).toList().first().toString(), QStringLiteral("actor-1"));
    EXPECT_EQ(models.contracts().data(contractIndex, ContractList::PropertyIdsRole).toList().first().toString(), QStringLiteral("property-1"));

    const QModelIndex statementIndex = models.statements().index(0, 0);
    EXPECT_EQ(models.statements().data(statementIndex, StatementList::IdRole).toString(), QStringLiteral("statement-1"));
    EXPECT_EQ(models.statements().data(statementIndex, StatementList::NameRole).toString(), QStringLiteral("January Statement"));

    const QModelIndex txIndex = models.transactions().index(0, 0);
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::IdRole).toString(), QStringLiteral("tx-1"));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::StatementIdRole).toString(), QStringLiteral("statement-1"));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::NameRole).toString(), QStringLiteral("Rent"));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::BookingDateRole).toString(), QStringLiteral("2026-01-05"));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::ValutaRole).toString(), QStringLiteral("EUR"));
    EXPECT_DOUBLE_EQ(models.transactions().data(txIndex, TransactionList::AmountRole).toDouble(), 1250.0);
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::StatusRole).toInt(), static_cast<int>(core::domain::Transaction::Status::Verified));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::ActorIdRole).toString(), QStringLiteral("actor-1"));
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::TypeRole).toString(), QStringLiteral("lease"));
    EXPECT_TRUE(models.transactions().data(txIndex, TransactionList::AllocatableRole).toBool());
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::PropertyIdsRole).toList().first().toString(), QStringLiteral("property-1"));

    const QModelIndex analysisIndex = models.analyses().index(0, 0);
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::IdRole).toString(), QStringLiteral("analysis-1"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::NameRole).toString(), QStringLiteral("Monthly Analysis"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::TypeRole).toString(), QStringLiteral("tabular"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::ConfigRole).toString(), QStringLiteral("{\"groupBy\":\"month\"}"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::FilterRole).toString(), QStringLiteral("{}"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::AdjustmentsRole).toString(), QStringLiteral("{\"actor-1\":19.25}"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::ExportFormatRole).toString(), QStringLiteral("csv"));
    EXPECT_TRUE(models.analyses().data(analysisIndex, AnalysisList::IncludeCalcAdjustmentsRole).toBool());
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::ExportStateRole).toString(), QStringLiteral("{}"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::SnapshotTransactionsRole).toString(), QStringLiteral("[\"tx-1\"]"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::CreatedAtRole).toString(), QStringLiteral("2026-01-01T08:00:00Z"));
    EXPECT_EQ(models.analyses().data(analysisIndex, AnalysisList::UpdatedAtRole).toString(), QStringLiteral("2026-01-02T08:00:00Z"));

    const QModelIndex annualIndex = models.annuals().index(0, 0);
    EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::IdRole).toString(), QStringLiteral("annual-1"));
    EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::NameRole).toString(), QStringLiteral("2026"));
    EXPECT_EQ(models.annuals().data(annualIndex, AnnualList::YearRole).toInt(), 2026);
}

TEST(SessionModelsTest, RefreshesTransactionTypesAfterContractChanges)
{
    auto catalog = tests::support::makeWorkspaceCatalog();
    auto contracts = catalog.contracts();
    contracts.front()->setType("rental");
    catalog.setContracts(std::move(contracts));

    SessionModels models;
    models.loadFromState(catalog);

    const QModelIndex txIndex = models.transactions().index(0, 0);
    EXPECT_EQ(models.transactions().data(txIndex, TransactionList::TypeRole).toString(), QStringLiteral("rental"));
}

} // namespace ui
