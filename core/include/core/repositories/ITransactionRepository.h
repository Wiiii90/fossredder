#pragma once

/**
 * @file core/include/core/repositories/ITransactionRepository.h
 * @brief Repository interface for Transaction persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Transaction;

class ITransactionRepository {
public:
    virtual ~ITransactionRepository() = default;

    /**
     * @brief Insert a new transaction into the repository.
     * @param transaction Shared pointer to the Transaction to add.
     */
    virtual void addTransaction(const std::shared_ptr<Transaction>& transaction) = 0;

    /**
     * @brief Return all transactions stored in the repository.
     * @return Vector of shared_ptr to Transaction.
     */
    virtual std::vector<std::shared_ptr<Transaction>> getTransactions() const = 0;

    /**
     * @brief Retrieve a transaction by its identifier.
     * @param id Transaction identifier string.
     * @return Optional shared_ptr to Transaction if found.
     */
    virtual std::optional<std::shared_ptr<Transaction>> getTransactionById(const std::string& id) const = 0;

    /**
     * @brief Remove a transaction identified by id from the repository.
     * @param id Transaction identifier to remove.
     */
    virtual void removeTransaction(const std::string& id) = 0;

    /**
     * @brief Update an existing transaction record in the repository.
     * @param transaction Shared pointer to the Transaction with updated fields.
     */
    virtual void updateTransaction(const std::shared_ptr<Transaction>& transaction) = 0;

    /**
     * @brief Upsert a transaction: insert or update depending on existence.
     * @param transaction Shared pointer to the Transaction to upsert.
     */
    virtual void upsertTransaction(const std::shared_ptr<Transaction>& transaction) = 0;

    /**
     * @brief Remove all transactions from the repository.
     */
    virtual void clearTransactions() = 0;
};
