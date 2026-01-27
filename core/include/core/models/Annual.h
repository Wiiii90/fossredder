#pragma once

#include <string>
#include <vector>
#include <algorithm>

class Annual {
public:
    Annual() = default;

    std::string id;
    int year = 0;

    // transaction ids included in this annual
    std::vector<std::string> transactionIds;

    // assigned analysis ids
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

    // helper
    void assignAnalysis(const std::string& analysisId) {
        for (const auto& a : assignedAnalysisIds) if (a == analysisId) return;
        assignedAnalysisIds.push_back(analysisId);
    }

    void unassignAnalysis(const std::string& analysisId) {
        assignedAnalysisIds.erase(std::remove(assignedAnalysisIds.begin(), assignedAnalysisIds.end(), analysisId), assignedAnalysisIds.end());
    }
};
