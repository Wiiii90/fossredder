#pragma once

#include "core/repositories/ITransactionRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteTransactionRepository : public ITransactionRepository {
public:
    explicit SqliteTransactionRepository(const std::string& dbPath);
    explicit SqliteTransactionRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteTransactionRepository() override;

    void addTransaction(const std::shared_ptr<Transaction>& transaction) override;
    std::vector<std::shared_ptr<Transaction>> getTransactions() const override;
    std::optional<std::shared_ptr<Transaction>> getTransactionById(const std::string& id) const override;
    void removeTransaction(const std::string& id) override;
    void updateTransaction(const std::shared_ptr<Transaction>& transaction) override;

    void upsertTransaction(const std::shared_ptr<Transaction>& transaction) override;
    void clearTransactions() override;

    std::vector<std::shared_ptr<Transaction>> getTransactionsForContract(const std::string& contractId) const override;
    void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
