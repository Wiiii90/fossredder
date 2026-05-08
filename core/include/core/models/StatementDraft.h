/**
 * @file core/include/core/models/StatementDraft.h
 * @brief Draft statement model used by import finalization and core application flow.
 */

#pragma once

#include <string>
#include <vector>

#include "core/models/TransactionDraft.h"

namespace core::domain {

class StatementDraft {
public:
    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
    std::vector<TransactionDraft> transactions;
};

} // namespace core::domain
