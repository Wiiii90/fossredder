/**
 * @file ui/tests/unit/TestAnalysisWorkflow.cpp
 * @brief Tests for analysis workflow transaction previews.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>

#include "core/application/analysis/AnalysisService.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "support/WorkspaceTestData.h"

namespace ui {

TEST(AnalysisWorkflowTest, PreviewTransactionsReturnsAllMatchingWorkspaceTransactions)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    const QVariantMap preview = workflow.previewTransactions(QStringLiteral(""));
    const QVariantList transactions = preview.value(QStringLiteral("transactions")).toList();
    const QVariantMap metrics = preview.value(QStringLiteral("metrics")).toMap();

    EXPECT_EQ(transactions.size(), 2);
    EXPECT_EQ(metrics.value(QStringLiteral("transactionCount")).toInt(), 2);
    EXPECT_EQ(metrics.value(QStringLiteral("statementCount")).toInt(), 1);

    ASSERT_FALSE(transactions.isEmpty());
    const QVariantMap first = transactions.front().toMap();
    EXPECT_EQ(first.value(QStringLiteral("id")).toString(), QStringLiteral("tx-1"));
    EXPECT_EQ(first.value(QStringLiteral("statementId")).toString(), QStringLiteral("statement-1"));
    EXPECT_EQ(first.value(QStringLiteral("contractType")).toString(), QStringLiteral("lease"));
}

TEST(AnalysisWorkflowTest, PreviewTransactionsHonorsAnalysisFilters)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    const QVariantMap preview = workflow.previewTransactions(QStringLiteral("amount>=1000"));
    const QVariantList transactions = preview.value(QStringLiteral("transactions")).toList();
    const QVariantMap metrics = preview.value(QStringLiteral("metrics")).toMap();

    EXPECT_EQ(transactions.size(), 1);
    EXPECT_EQ(metrics.value(QStringLiteral("transactionCount")).toInt(), 1);
    ASSERT_FALSE(transactions.isEmpty());
    EXPECT_EQ(transactions.front().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("tx-1"));
}

TEST(AnalysisWorkflowTest, AnalysisFilterSpecUsesYearDefaultAndOmitsAllSelectedGroups)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    const int defaultYear = QDate::currentDate().year() - 1;
    const QString filterSpec = workflow.analysisFilterSpec(QStringLiteral("bookingDate"),
                                                           QStringLiteral("year"),
                                                           QString(),
                                                           QString(),
                                                           QString(),
                                                           QStringList{},
                                                           QStringList{},
                                                           QStringLiteral("all"));

    EXPECT_EQ(filterSpec,
              QStringLiteral("date>=%1-01-01;date<=%2-12-31")
                  .arg(defaultYear)
                  .arg(defaultYear));
}

TEST(AnalysisWorkflowTest, AnalysisFilterSpecEmitsExplicitClausesForPartialSelections)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    const QString filterSpec = workflow.analysisFilterSpec(QStringLiteral("bookingDate"),
                                                           QStringLiteral("range"),
                                                           QStringLiteral("2025"),
                                                           QStringLiteral("2025-01-01"),
                                                           QStringLiteral("2025-12-31"),
                                                           QStringList{QStringLiteral("property-1")},
                                                           QStringList{QStringLiteral("lease")},
                                                           QStringLiteral("allocatable"));

    EXPECT_EQ(filterSpec,
              QStringLiteral("date>=2025-01-01;date<=2025-12-31;propertyId=property-1;contract.type=lease;allocatable=allocatable"));
}

TEST(AnalysisWorkflowTest, AnalysisConfigJsonUsesCoreAnalysisKeys)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    const QString json = workflow.analysisConfigJson(QStringLiteral("plot"),
                                                     QStringLiteral("histogram"),
                                                     QStringLiteral("totalAmount"),
                                                     QStringList{QStringLiteral("property-1")},
                                                     QStringList{QStringLiteral("lease")},
                                                     0.0);

    const QJsonObject parsed = QJsonDocument::fromJson(json.toUtf8()).object();
    EXPECT_EQ(parsed.value(QStringLiteral("plotType")).toString(), QStringLiteral("histogram"));
    EXPECT_EQ(parsed.value(QStringLiteral("plotMeasure")).toString(), QStringLiteral("totalAmount"));
    ASSERT_TRUE(parsed.value(QStringLiteral("properties")).isArray());
    EXPECT_EQ(parsed.value(QStringLiteral("properties")).toArray().at(0).toString(), QStringLiteral("property-1"));
    ASSERT_TRUE(parsed.value(QStringLiteral("contractTypes")).isArray());
    EXPECT_EQ(parsed.value(QStringLiteral("contractTypes")).toArray().at(0).toString(), QStringLiteral("lease"));
}

TEST(AnalysisWorkflowTest, AnalysisAdjustmentsJsonStoresAdjustedAmountsByTransactionId)
{
    auto state = std::make_shared<const core::domain::catalog::WorkspaceCatalog>(tests::support::makeWorkspaceCatalog());
    auto analysisService = std::make_shared<core::application::analysis::AnalysisService>();
    AnalysisWorkflow workflow(nullptr, [state]() { return state; }, analysisService);

    QVariantMap selected;
    selected.insert(QStringLiteral("id"), QStringLiteral("tx-1"));
    selected.insert(QStringLiteral("amount"), 100.0);
    QVariantMap ignored;
    ignored.insert(QStringLiteral("id"), QStringLiteral("tx-2"));
    ignored.insert(QStringLiteral("amount"), 50.0);

    const QString json = workflow.analysisAdjustmentsJson(QVariantList{selected, ignored},
                                                          QStringList{QStringLiteral("tx-1")},
                                                          19.0);

    const QJsonObject parsed = QJsonDocument::fromJson(json.toUtf8()).object();
    EXPECT_DOUBLE_EQ(parsed.value(QStringLiteral("tx-1")).toDouble(), 119.0);
    EXPECT_FALSE(parsed.contains(QStringLiteral("tx-2")));
}

} // namespace ui
