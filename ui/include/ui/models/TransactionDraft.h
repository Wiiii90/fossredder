#pragma once

#include <QString>
#include <QStringList>

#include "ui/models/ImportSuggestion.h"
#include "core/models/Transaction.h"

namespace ui {

struct TransactionDraft {
    QString name;
    QString bookingDate;
    QString valuta;
    double amount = 0.0;

    QString description;

    QString actorText;
    QString propertyText;
    QString actorId;
    bool newActorSelected = false;
    QString contractId;
    bool newContractSelected = false;

    QString metadata;
    QString proofImagePath;

    QString type;

    ImportDraftSuggestions suggestions;

    bool allocatable = false;
    bool allocatableManualOverride = false;

    // Default to Unverified so new transactions require user verification
    int status = static_cast<int>(core::domain::Transaction::Status::Unverified);

    // Associated property ids (UI-level)
    QStringList propertyIds;
};

}
