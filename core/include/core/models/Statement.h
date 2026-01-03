#pragma once

#include <string>
#include <vector>

#include "core/models/Transaction.h"

class Statement {
public:
    Statement() = default;
    explicit Statement(std::vector<Transaction> transactions);

    void addTransaction(const Transaction& t);
    bool removeTransaction(const Transaction& t);

    std::string id;
    std::string name;
    std::vector<Transaction> transactions;
};
