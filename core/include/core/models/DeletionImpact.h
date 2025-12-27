#pragma once

#include <string>
#include <vector>

struct DeletionImpact {
    std::vector<std::string> deletedActorIds;
    std::vector<std::string> deletedPropertyIds;
    std::vector<std::string> deletedContractIds;
    std::vector<std::string> deletedStatementIds;
    std::vector<std::string> deletedTransactionIds;

    bool empty() const noexcept {
        return deletedActorIds.empty() && deletedPropertyIds.empty() && deletedContractIds.empty() &&
               deletedStatementIds.empty() && deletedTransactionIds.empty();
    }
};
