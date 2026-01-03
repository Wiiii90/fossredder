#pragma once

#include <string>

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
                std::string description = {});

    std::string id;
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;

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

    bool operator==(const Transaction& other) const noexcept;
};
