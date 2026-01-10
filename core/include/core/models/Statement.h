#pragma once

#include <string>
#include <vector>
#include <memory>

class Transaction;

class Statement {
public:
    Statement() = default;

    std::string id;
    std::string name;

    // Associated transactions (shared pointers into global transaction store)
    std::vector<std::shared_ptr<Transaction>> transactions;
};
