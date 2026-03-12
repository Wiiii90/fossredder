/**
 * @file persistence/include/persistence/repositories/SqliteTransactionRepository.h
 * @brief Declares the SQLite-backed transaction repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/repositories/ITransactionRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteTransactionRepository : public ITransactionRepository {
public:
    explicit SqliteTransactionRepository(const std::string& dbPath);
    SqliteTransactionRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    explicit SqliteTransactionRepository(std::shared_ptr<SqliteDb> db);
    SqliteTransactionRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    ~SqliteTransactionRepository() override;

    void addTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;
    std::vector<std::shared_ptr<core::domain::Transaction>> getTransactions() const override;
    std::optional<std::shared_ptr<core::domain::Transaction>> getTransactionById(const std::string& id) const override;
    void removeTransaction(const std::string& id) override;
    void updateTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;

    void upsertTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) override;
    void clearTransactions() override;

    std::vector<std::shared_ptr<core::domain::Transaction>> getTransactionsForContract(const std::string& contractId) const override;
    void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
