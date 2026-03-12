#include "gtest/gtest.h"

#include "core/application/AppStateManager.h"
#include "core/models/AppState.h"

#include "core/models/Analysis.h"
#include "core/models/Annual.h"

#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/storage/RepositoryBundle.h"

using core::application::AppStateManager;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::AppState;

class FakeAnalysisRepository : public IAnalysisRepository {
public:
    std::vector<std::shared_ptr<Analysis>> analyses;
    bool upsertCalled = false;

    void addAnalysis(const std::shared_ptr<Analysis>& analysis) override { analyses.push_back(analysis); }
    std::vector<std::shared_ptr<Analysis>> getAnalyses() const override { return analyses; }
    std::optional<std::shared_ptr<Analysis>> getAnalysisById(const std::string& id) const override {
        for (const auto& analysis : analyses) if (analysis && analysis->id == id) return analysis;
        return std::nullopt;
    }
    void removeAnalysis(const std::string&) override {}
    void updateAnalysis(const std::shared_ptr<Analysis>&) override {}
    void upsertAnalysis(const std::shared_ptr<Analysis>& analysis) override { upsertCalled = true; analyses.push_back(analysis); }
    void clearAnalyses() override { analyses.clear(); }
};

class FakeAnnualRepository : public IAnnualRepository {
public:
    std::vector<std::shared_ptr<Annual>> annuals;
    bool upsertCalled = false;

    void addAnnual(const std::shared_ptr<Annual>& annual) override { annuals.push_back(annual); }
    std::vector<std::shared_ptr<Annual>> getAnnuals() const override { return annuals; }
    std::optional<std::shared_ptr<Annual>> getAnnualById(const std::string& id) const override {
        for (const auto& annual : annuals) if (annual && annual->id == id) return annual;
        return std::nullopt;
    }
    void removeAnnual(const std::string&) override {}
    void updateAnnual(const std::shared_ptr<Annual>&) override {}
    void upsertAnnual(const std::shared_ptr<Annual>& annual) override { upsertCalled = true; annuals.push_back(annual); }
    void clearAnnuals() override { annuals.clear(); }
};

TEST(AppStateManagerExtendedTests, LoadIncludesAnalysesAndAnnuals)
{
    AppStateManager::Repositories repos;
    auto analysisRepo = std::make_shared<FakeAnalysisRepository>();
    auto annualRepo = std::make_shared<FakeAnnualRepository>();

    auto analysis = std::make_shared<Analysis>();
    analysis->id = "AN-1";
    analysisRepo->analyses.push_back(analysis);

    auto annual = std::make_shared<Annual>();
    annual->id = "YR-1";
    annual->year = 2024;
    annualRepo->annuals.push_back(annual);

    repos.analyses = analysisRepo;
    repos.annuals = annualRepo;

    AppStateManager manager(std::move(repos));
    const AppState state = manager.load();

    ASSERT_EQ(state.analyses.size(), 1u);
    ASSERT_EQ(state.annuals.size(), 1u);
    EXPECT_EQ(state.analyses.front()->id, "AN-1");
    EXPECT_EQ(state.annuals.front()->year, 2024);
}

TEST(AppStateManagerExtendedTests, SaveUpsertsAnalysesAndAnnuals)
{
    AppStateManager::Repositories repos;
    auto analysisRepo = std::make_shared<FakeAnalysisRepository>();
    auto annualRepo = std::make_shared<FakeAnnualRepository>();

    repos.analyses = analysisRepo;
    repos.annuals = annualRepo;

    AppStateManager manager(std::move(repos));

    AppState state;
    auto analysis = std::make_shared<Analysis>();
    analysis->id = "AN-2";
    analysis->type = "tab";
    state.analyses.push_back(analysis);

    auto annual = std::make_shared<Annual>();
    annual->id = "YR-2";
    annual->year = 2025;
    state.annuals.push_back(annual);

    manager.save(state);

    EXPECT_TRUE(analysisRepo->upsertCalled);
    EXPECT_TRUE(annualRepo->upsertCalled);
}
