#pragma once
#include <string>
#include <map>

class Transaction {
public:
    Transaction();
    Transaction(const std::string& bookingDate,
                const std::string& valutaDate,
                const std::string& details,
                const std::string& amountText);

    Transaction(const Transaction& other);

    std::string bookingDate;
    std::string valutaDate;
    std::string details;
    std::string amountText;
    double amount;
    std::map<std::string, std::string> extractedFields;

    double parseAmount(const std::string& text) const;
    std::string getBookingDate() const;
    double getAmount() const;
};