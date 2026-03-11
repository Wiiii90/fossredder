/**
 * @file core/include/core/models/Annual.h
 * @brief Domain model for annual aggregates.
 */

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace core::domain {

class Annual {
public:
    Annual() = default;

    std::string id;
    int year = 0;
    std::vector<std::string> transactionIds;
    std::vector<std::string> assignedAnalysisIds;

    enum class VerificationState : int {
        Draft = 0,
        Verified = 1,
        Locked = 2
    };

    VerificationState verificationState = VerificationState::Draft;
    std::string createdAt;
    std::string updatedAt;
    int schemaVersion = 1;

    void assignAnalysis(const std::string& analysisId) {
        for (const auto& a : assignedAnalysisIds) if (a == analysisId) return;
        assignedAnalysisIds.push_back(analysisId);
    }

    void unassignAnalysis(const std::string& analysisId) {
        assignedAnalysisIds.erase(std::remove(assignedAnalysisIds.begin(), assignedAnalysisIds.end(), analysisId), assignedAnalysisIds.end());
    }
};

}

using Annual = core::domain::Annual;
