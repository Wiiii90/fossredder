#pragma once

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

#include "Transaction.h"
#include "Header.h"

class BookingGroup {
public:
    BookingGroup() = default;
    explicit BookingGroup(std::vector<Transaction> txs);

    void addTransaction(Transaction t);
    // remove returns false if would violate invariant (must keep >=1 transaction)
    bool removeTransactionAt(size_t idx);

    double totalAmount() const noexcept;

    // find transaction index by metadata id key "id" (returns nullopt if not found or id empty)
    std::optional<size_t> findIndexById(const std::string& id) const noexcept;

    const std::vector<Transaction>& getTransactions() const noexcept { return transactions; }

    std::string title;
    // Composition: BookingGroup owns its Transactions
    std::vector<Transaction> transactions;
};
