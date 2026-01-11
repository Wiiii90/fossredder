#pragma once

#include <string>
#include <vector>

class Actor;
class Contract;
class Statement;

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
                Contract* contract = nullptr,
                Actor* actor = nullptr,
                std::string description = {},
                bool allocatable = false);

    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;

    // New: transaction type (free text)
    std::string type;

    Status status = Status::Neutral;

    std::string contractId;
    Contract* contract = nullptr;

    std::string actorId;
    Actor* actor = nullptr;

    std::string statementId;

    // Raw/legacy free text (still kept for backward compatibility and UI display)
    std::string description;

    // OCR/import suggestion
    std::string actorProposal;

    // Geometry-based OCR metadata block (read-only, derived during import).
    std::string metadata;

    // Optional image path for user verification (crop of the original statement page).
    std::string proofImagePath;

    // Whether this transaction's costs are allocatable to tenants (umlegbar)
    bool allocatable = false;

    // Associated properties (many-to-many)
    std::vector<std::string> propertyIds;

    bool operator==(const Transaction& other) const noexcept;
};
