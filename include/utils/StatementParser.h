#pragma once

#include <vector>
#include <memory>
#include <string>

#include "models/StatementData.h"

class Transaction;

namespace utils {

class StatementParser {
public:
    // Parse transactions from a complete StatementData (contains layout + OCR artifacts)
    static std::vector<std::shared_ptr<Transaction>> parseTransactions(const StatementData& statementData);
};

} // namespace utils
