#include "gtest/gtest.h"

#include "core/analysis/AnalysisEngine.h"
#include "core/constants/CoreDefaults.h"
#include "core/models/Analysis.h"
#include "core/models/AnalysisResult.h"
#include "core/models/AppState.h"

TEST(AnalysisEngineTests, UnknownTypeReturnsNotFound)
{
    core::analysis::AnalysisEngine engine;
    Analysis analysis;
    analysis.type = "unknown";

    const AnalysisResult out = engine.computeAnalysis(analysis, AppState{});

    EXPECT_FALSE(out.found);
    EXPECT_TRUE(out.type.empty());
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
