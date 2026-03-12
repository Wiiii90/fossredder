#include "gtest/gtest.h"

#include "core/analysis/AnalysisEngine.h"
#include "core/constants/CoreDefaults.h"
#include "core/models/Analysis.h"
#include "core/models/AnalysisResult.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

using core::domain::Analysis;
using core::domain::AnalysisResult;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Transaction;

TEST(AnalysisEngineTests, UnknownTypeFallsBackToTabStrategy)
{
    core::analysis::AnalysisEngine engine;
    Analysis analysis;
    analysis.type = "unknown";

    const AnalysisResult out = engine.computeAnalysis(analysis, AppState{});

    EXPECT_TRUE(out.found);
    EXPECT_EQ(out.type, std::string("unknown"));
}

TEST(AnalysisEngineTests, PlotConfigResolvesOutputType)
{
    core::analysis::AnalysisEngine engine;
    Analysis analysis;
    analysis.type = std::string(core::constants::analysis::kTypePlot);
    analysis.configJson = R"({"plotType":"bar"})";

    const AnalysisResult out = engine.computeAnalysis(analysis, AppState{});

    EXPECT_TRUE(out.found);
    EXPECT_EQ(out.type, "bar");
}

TEST(AnalysisEngineTests, PlotPieAggregatesTransactionsByContractType)
{
    core::analysis::AnalysisEngine engine;

    AppState state;

    auto rent = std::make_shared<Contract>();
    rent->id = "C1";
    rent->type = "Rent";
    state.contracts.push_back(rent);

    auto utilities = std::make_shared<Contract>();
    utilities->id = "C2";
    utilities->type = "Utilities";
    state.contracts.push_back(utilities);

    auto tx1 = std::make_shared<Transaction>();
    tx1->id = "T1";
    tx1->amount = 100.0;
    tx1->contractId = "C1";
    state.transactions.push_back(tx1);

    auto tx2 = std::make_shared<Transaction>();
    tx2->id = "T2";
    tx2->amount = 50.0;
    tx2->contractId = "C1";
    state.transactions.push_back(tx2);

    auto tx3 = std::make_shared<Transaction>();
    tx3->id = "T3";
    tx3->amount = 20.0;
    tx3->contractId = "C2";
    state.transactions.push_back(tx3);

    Analysis analysis;
    analysis.type = std::string(core::constants::analysis::kTypePlot);
    analysis.configJson = R"({"plotType":"pie","plotMeasure":"totalAmount"})";

    const AnalysisResult out = engine.computeAnalysis(analysis, state);

    EXPECT_TRUE(out.found);
    EXPECT_EQ(out.type, std::string(core::constants::analysis::plotTypes::kPie));
    EXPECT_DOUBLE_EQ(out.metrics.at(std::string(core::constants::analysis::metricKeys::kTotalAmount)), 170.0);
    ASSERT_EQ(out.table.size(), 2u);
}
