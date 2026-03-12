/**
 * @file core/include/core/models/DraftStatement.h
 * @brief Transient draft models for imported statements before they become domain entities.
 */
#pragma once

#include "core/models/Transaction.h"

#include <string>
#include <vector>

namespace core::domain {

struct DraftTransaction {
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string description;
    Transaction::Status status = Transaction::Status::Neutral;
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

