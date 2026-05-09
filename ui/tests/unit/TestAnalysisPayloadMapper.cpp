/**
 * @file ui/tests/unit/TestAnalysisPayloadMapper.cpp
 * @brief Unit tests for `AnalysisPayloadMapper`.
 */

#include "gtest/gtest.h"

#include "core/application/analysis/RunAnalysisResult.h"
#include "ui/analysis/AnalysisPayloadMapper.h"
#include "ui/payload/PayloadKeys.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/text/Text.h"

namespace {

using core::domain::AnalysisResult;
using core::domain::AnalysisTransaction;

TEST(AnalysisPayloadMapperTests, ToPayloadMapsNestedAnalysisResultFields)
{
    AnalysisResult result;
    result.type = "calc";
    result.configJson = R"({"mode":"tax"})";
    result.metrics = {{"total", 12.5}, {"count", 3.0}};
    result.table = {{"month", "total"}, {"2025-01", "12.5"}};
    result.artifacts = {"report.csv", "report.json"};
    result.generatedAt = "2025-01-15T12:00:00Z";
    result.transactions = {
        AnalysisTransaction{"tx-1", "Rent", "2025-01-01", 12.5, "contract-1", ""},
        AnalysisTransaction{"tx-2", "Water", "2025-01-02", 5.0, "contract-2", "utility"}
    };

    const QVariantMap payload = ui::analysis::toPayload(result);

    ASSERT_EQ(payload.value(ui::payload::keys::common::kType).toString(), QStringLiteral("calc"));
    ASSERT_EQ(payload.value(ui::payload::keys::analysis::kConfig).toString(), QStringLiteral(R"({"mode":"tax"})"));
    ASSERT_EQ(payload.value(ui::payload::keys::analysis::kGeneratedAt).toString(), QStringLiteral("2025-01-15T12:00:00Z"));

    const QVariantMap metrics = payload.value(ui::payload::keys::analysis::kMetrics).toMap();
    EXPECT_DOUBLE_EQ(metrics.value(QStringLiteral("total")).toDouble(), 12.5);
    EXPECT_DOUBLE_EQ(metrics.value(QStringLiteral("count")).toDouble(), 3.0);

    const QVariantList table = payload.value(ui::payload::keys::analysis::kTable).toList();
    ASSERT_EQ(table.size(), 2);
    EXPECT_EQ(table.at(0).toList().at(0).toString(), QStringLiteral("month"));
    EXPECT_EQ(table.at(1).toList().at(1).toString(), QStringLiteral("12.5"));

    const QVariantList artifacts = payload.value(ui::payload::keys::analysis::kArtifacts).toList();
    ASSERT_EQ(artifacts.size(), 2);
    EXPECT_EQ(artifacts.at(0).toString(), QStringLiteral("report.csv"));
    EXPECT_EQ(artifacts.at(1).toString(), QStringLiteral("report.json"));

    const QVariantList transactions = payload.value(ui::payload::keys::analysis::kTransactions).toList();
    ASSERT_EQ(transactions.size(), 2);

    const QVariantMap firstTransaction = transactions.at(0).toMap();
    EXPECT_EQ(firstTransaction.value(ui::payload::keys::common::kId).toString(), QStringLiteral("tx-1"));
    EXPECT_EQ(firstTransaction.value(ui::payload::keys::transaction::kContractType).toString(),
              ui::text::analysis::unassignedContractType());

    const QVariantMap secondTransaction = transactions.at(1).toMap();
    EXPECT_EQ(secondTransaction.value(ui::payload::keys::transaction::kContractType).toString(), QStringLiteral("utility"));
    EXPECT_DOUBLE_EQ(secondTransaction.value(ui::payload::keys::common::kAmount).toDouble(), 5.0);
}

} // namespace

