#pragma once

#include <string>
#include <vector>
#include <optional>

#include "core/models/BookingGroup.h"
#include "core/models/Transaction.h"

class Statement {
public:
    Statement() = default;
    explicit Statement(std::vector<BookingGroup> groups);

    void addBookingGroup(BookingGroup g);
    bool removeBookingGroupAt(size_t idx);

    double totalAmount() const noexcept;

    std::vector<Transaction> flattenTransactions() const;

    std::optional<size_t> findGroupByTitle(const std::string& title) const noexcept;

    const std::vector<BookingGroup>& getBookingGroups() const noexcept { return bookingGroups; }

    std::string sourceFile;
    std::vector<BookingGroup> bookingGroups;
};
