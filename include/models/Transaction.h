#pragma once
#include <string>

class Transaction {
public:
    std::string bookingDate;
    std::string valutaDate;
    std::string actor;
    std::string description;
    double amount;
    bool isDebit;

    Transaction(const std::string& bookingDate, const std::string& valutaDate,
                const std::string& actor, const std::string& description,
                double amount, bool isDebit);
};
