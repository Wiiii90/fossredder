#pragma once

#include <string>
#include <vector>
#include <optional>
#include <numeric>

#include "BookingGroup.h"
#include "Transaction.h"
#include "Header.h"

class Statement {
public:
    Statement() = default;
    explicit Statement(std::vector<BookingGroup> groups);

    void addBookingGroup(BookingGroup g);
    bool removeBookingGroupAt(size_t idx);

    double totalAmount() const noexcept;

    // Flatten all transactions from all booking groups into a single vector
    std::vector<Transaction> flattenTransactions() const;

    std::optional<size_t> findGroupByTitle(const std::string& title) const noexcept;

    const std::vector<BookingGroup>& getBookingGroups() const noexcept { return bookingGroups; }

    std::string sourceFile;
    // Composition: Statement owns its BookingGroups
    std::vector<BookingGroup> bookingGroups;
};
