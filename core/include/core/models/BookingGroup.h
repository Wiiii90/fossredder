#pragma once

#include "core/models/Transaction.h"
#include <vector>
#include <optional>

class BookingGroup {
public:
    BookingGroup() = default;
    explicit BookingGroup(std::vector<Transaction> txs);

    void addTransaction(Transaction t);
    bool removeTransactionAt(size_t idx);

    double totalAmount() const noexcept;

    std::optional<size_t> findIndexById(const std::string& id) const noexcept;

    const std::vector<Transaction>& getTransactions() const noexcept { return transactions; }

    std::string title;
    std::vector<Transaction> transactions;
};
