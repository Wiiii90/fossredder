#pragma once

#include <string>
#include <map>

class Transaction {
public:
    Transaction() noexcept = default;
    Transaction(std::string bookingDate,
                std::string valueDate,
                double amount,
                std::string actor = {},
                std::map<std::string, std::string> metadata = {});
    Transaction(const Transaction&) = default;
    Transaction& operator=(const Transaction&) = default;

    std::string bookingDate;
    std::string valueDate;
    double amount = 0.0;
    std::string actor;
    std::map<std::string, std::string> metadata;

    bool isDebit() const noexcept;
    bool isCredit() const noexcept;

    std::string toString() const;
};
