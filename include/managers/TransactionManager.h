#pragma once
#include "managers/ITransactionManager.h"

class Transaction; // forward declaration - avoid including models headers here

class TransactionManager : public ITransactionManager {
public:
    void addTransaction(const std::shared_ptr<Transaction>& transaction) override;
    std::vector<std::shared_ptr<Transaction>> getTransactions() const override;
    void removeTransaction(const std::string& id) override;
    void updateTransaction(const std::shared_ptr<Transaction>& transaction) override;
private:
    std::vector<std::shared_ptr<Transaction>> transactions_;
};