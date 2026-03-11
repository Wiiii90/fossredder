#pragma once

#include <string>
#include <vector>

namespace core::domain {

struct DraftTransaction {
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string description;
    int status = 0;
    std::string actorId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
    std::string type;
};

struct DraftStatement {
    std::string name;
    std::vector<DraftTransaction> transactions;
};

}

using DraftTransaction = core::domain::DraftTransaction;
using DraftStatement = core::domain::DraftStatement;
