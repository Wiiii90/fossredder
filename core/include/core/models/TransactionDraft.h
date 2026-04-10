/**
 * @file core/include/core/models/TransactionDraft.h
 * @brief Draft transaction model used by import finalization and core application flow.
 */

#pragma once

#include <string>
#include <vector>

#include "core/models/Transaction.h"

namespace core::domain {

class TransactionDraft {
public:
    std::string id;
    std::string statementDraftId;
    int position = 0;

    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;

    std::string description;

    std::string actorText;
    std::string propertyText;
    std::string actorId;
    bool newActorSelected = false;
    std::string contractId;
    bool newContractSelected = false;

    std::string metadata;
    std::string proofImagePath;

    std::string type;

    bool allocatable = false;
    bool allocatableManualOverride = false;

    Transaction::Status status = Transaction::Status::Unverified;

    std::vector<std::string> propertyIds;
};

} // namespace core::domain
