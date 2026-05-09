#pragma once

#include <map>
#include <string>
#include <vector>

namespace core::ports::presenters {

struct AnalysisTransactionPresentation {
    std::string id;
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string contractId;
    std::string contractType;
    std::vector<std::string> propertyIds;
};

struct AnalysisPresentation {
    std::string type;
    std::string configJson;
    std::map<std::string, double> metrics;
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> artifacts;
    std::vector<AnalysisTransactionPresentation> transactions;
    bool found = false;
    std::string generatedAt;
};

class IAnalysisPresenter {
public:
    virtual ~IAnalysisPresenter() = default;

    virtual AnalysisPresentation present(const AnalysisPresentation& result) const = 0;
};

} // namespace core::ports::presenters
