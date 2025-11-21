#pragma once

#include "core/repositories/ITransactionRepository.h"
#include <memory>
#include <string>

class SqliteTransactionRepository : public ITransactionRepository {
public:
    explicit SqliteTransactionRepository(const std::string& dbPath);
    ~SqliteTransactionRepository() override;

    void addTransaction(const std::shared_ptr<Transaction>& transaction) override;
    std::vector<std::shared_ptr<Transaction>> getTransactions() const override;
    std::optional<std::shared_ptr<Transaction>> getTransactionById(const std::string& id) const override;
    void removeTransaction(const std::string& id) override;
    void updateTransaction(const std::shared_ptr<Transaction>& transaction) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
