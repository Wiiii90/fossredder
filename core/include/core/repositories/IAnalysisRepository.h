#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class core::domain::Analysis;
}

class IAnalysisRepository {
public:
    virtual ~IAnalysisRepository() = default;

    virtual void addAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;
    virtual std::vector<std::shared_ptr<core::domain::Analysis>> getAnalyses() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::Analysis>> getAnalysisById(const std::string& id) const = 0;
    virtual void removeAnalysis(const std::string& id) = 0;
    virtual void updateAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;
    virtual void upsertAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;
    virtual void clearAnalyses() = 0;
};
