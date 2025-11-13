#pragma once

#include <string>
#include <map>

// Simple Transaction model used by views/managers during refactor.
// Fields are kept as strings for dates to avoid timezone/chrono complexity.
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

    // Core fields
    std::string bookingDate;   // booking/posting date (as string)
    std::string valueDate;     // value/valuta date (as string)
    double amount = 0.0;       // signed amount: positive = credit/incoming, negative = debit/outgoing
    std::string actor;         // payee/payer

    // Details are now modeled as flexible key/value metadata
    std::map<std::string, std::string> metadata; // e.g. {"creditor_id": "DE...", "customer_ref": "..."}

    // Helpers
    bool isDebit() const noexcept;   // amount < 0
    bool isCredit() const noexcept;  // amount > 0

    // Debug helper
    std::string toString() const;
};
