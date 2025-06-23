#pragma once
#include <string>

class Transaction {
public:
    Transaction(const std::string& bookingDate,
        const std::string& valutaDate,
        const std::string& details,
        const std::string& amountText);

    std::string bookingDate;
    std::string valutaDate;
    std::string details;
    std::string amountText;
    double amount;

    double parseAmount(const std::string& text) const;
    std::string getBookingDate() const { return bookingDate; }
    double getAmount() const { return amount; }
};