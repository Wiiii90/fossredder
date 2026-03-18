#pragma once

/**
 * @file core/include/core/repositories/ITransactionRepository.h
 * @brief Repository interface for core::domain::Transaction persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Transaction;
}

class ITransactionRepository {
public:
    virtual ~ITransactionRepository() = default;

    /**
     * @brief Insert a new transaction into the repository.
     * @param transaction Shared pointer to the core::domain::Transaction to add.
     */
    virtual void addTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Return all transactions stored in the repository.
     * @return Vector of shared_ptr to core::domain::Transaction.
     */
    virtual std::vector<std::shared_ptr<core::domain::Transaction>> getTransactions() const = 0;

    /**
     * @brief Retrieve a transaction by its identifier.
     * @param id core::domain::Transaction identifier string.
     * @return Optional shared_ptr to core::domain::Transaction if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Transaction>> getTransactionById(const std::string& id) const = 0;

    /**
     * @brief Remove a transaction identified by id from the repository.
     * @param id core::domain::Transaction identifier to remove.
     */
    virtual void removeTransaction(const std::string& id) = 0;

    /**
     * @brief Update an existing transaction record in the repository.
     * @param transaction Shared pointer to the core::domain::Transaction with updated fields.
     */
    virtual void updateTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Upsert a transaction: insert or update depending on existence.
     * @param transaction Shared pointer to the core::domain::Transaction to upsert.
     */
    virtual void upsertTransaction(const std::shared_ptr<core::domain::Transaction>& transaction) = 0;

    /**
     * @brief Remove all transactions from the repository.
     */
    virtual void clearTransactions() = 0;

    /**
     * @brief Return transactions associated with a given contract id.
     */
    virtual std::vector<std::shared_ptr<core::domain::Transaction>> getTransactionsForContract(const std::string& contractId) const = 0;

    /**
     * @brief Assign a list of transactions to a contract (bulk operation).
     * @param contractId core::domain::Contract identifier to assign to. Empty string to clear.
     * @param transactionIds List of transaction id strings to assign.
     */
    virtual void assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) = 0;
};
