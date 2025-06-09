#pragma once
#include <memory>
#include <vector>
#include <string>

class Transaction;

class ITransactionManager {
public:
    virtual ~ITransactionManager() = default;
    virtual void addTransaction(const std::shared_ptr<Transaction>& transaction) = 0;
    virtual std::vector<std::shared_ptr<Transaction>> getTransactions() const = 0;
    virtual void removeTransaction(const std::string& id) = 0;
    virtual void updateTransaction(const std::shared_ptr<Transaction>& transaction) = 0;
};