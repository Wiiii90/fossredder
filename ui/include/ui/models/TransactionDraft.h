#pragma once

#include <QString>
#include "core/models/Transaction.h"

struct TransactionDraft {
    QString name;
    QString bookingDate;
    QString valuta;
    double amount = 0.0;

    QString description;

    QString actorId;
    QString actorProposal;

    QString metadata;
    QString proofImagePath;

    bool allocatable = false;

    // Default to Unverified so new transactions require user verification
    int status = static_cast<int>(Transaction::Status::Unverified);
};
