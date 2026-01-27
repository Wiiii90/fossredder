#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Analysis;

class IAnalysisRepository {
public:
    virtual ~IAnalysisRepository() = default;

    virtual void addAnalysis(const std::shared_ptr<Analysis>& analysis) = 0;
    virtual std::vector<std::shared_ptr<Analysis>> getAnalyses() const = 0;
    virtual std::optional<std::shared_ptr<Analysis>> getAnalysisById(const std::string& id) const = 0;
    virtual void removeAnalysis(const std::string& id) = 0;
    virtual void updateAnalysis(const std::shared_ptr<Analysis>& analysis) = 0;
    virtual void upsertAnalysis(const std::shared_ptr<Analysis>& analysis) = 0;
    virtual void clearAnalyses() = 0;
};
