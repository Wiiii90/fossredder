/**
 * @file core/include/core/models/Transaction.h
 * @brief Domain model for transactions.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace core::domain {

class Transaction {
public:
    enum class Status : int {
        Neutral = 0,
        Unverified = 1,
        Verified = 2,
        Completed = 3
    };

    Transaction() noexcept = default;
    Transaction(std::string name,
                std::string bookingDate,
                std::string valuta,
                double amount,
                std::string description = {},
                bool allocatable = false)
        : name(std::move(name)),
          bookingDate(std::move(bookingDate)),
          valuta(std::move(valuta)),
          amount(amount),
          description(std::move(description)),
          allocatable(allocatable) {}

    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    Status status = Status::Neutral;
    std::string contractId;
    std::string actorId;
    std::string statementId;
    std::string description;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
};

}

