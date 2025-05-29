#pragma once
#include <string>
#include <vector>
#include "models/Transaction.h"

class BookingGroup {
public:
    BookingGroup(const std::string& bookingDate, std::vector<Transaction>&& transactions);

    const std::string& getBookingDate() const;
    const std::vector<Transaction>& getTransactions() const;

    static double parseAmount(const std::string& text);
    static std::vector<Transaction> extractTransactions(const std::vector<Header>& headers);

private:
    std::string bookingDate;
    std::vector<Transaction> transactions;
};