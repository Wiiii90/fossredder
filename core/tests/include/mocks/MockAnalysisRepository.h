#pragma once

#include "core/repositories/IAnalysisRepository.h"
#include <gmock/gmock.h>

using core::domain::Analysis;

class MockAnalysisRepository : public IAnalysisRepository {
public:
    MOCK_METHOD(void, addAnalysis, (const std::shared_ptr<Analysis>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Analysis>>, getAnalyses, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Analysis>>, getAnalysisById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeAnalysis, (const std::string&), (override));
    MOCK_METHOD(void, updateAnalysis, (const std::shared_ptr<Analysis>&), (override));
    MOCK_METHOD(void, upsertAnalysis, (const std::shared_ptr<Analysis>&), (override));
    MOCK_METHOD(void, clearAnalyses, (), (override));
};
