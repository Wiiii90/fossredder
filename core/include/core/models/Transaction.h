#pragma once

#include <string>

class Actor;
class Contract;
class Statement;

class Transaction {
public:
    Transaction() noexcept = default;
    Transaction(std::string name,
                std::string bookingDate,
                std::string valuta,
                double amount,
                Contract* contract = nullptr,
                Actor* actor = nullptr,
                std::string description = {});

    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;

    std::string contractId;
    Contract* contract = nullptr;

    std::string actorId;
    Actor* actor = nullptr;

    std::string statementId;

    std::string description;

    bool operator==(const Transaction& other) const noexcept;
};
