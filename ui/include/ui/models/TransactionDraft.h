/**
 * @file ui/include/ui/models/TransactionDraft.h
 * @brief Declarations for the UI TransactionDraft component.
 */

#pragma once

#include <QString>
#include <QStringList>

#include "ui/models/ImportSuggestion.h"
#include "core/domain/entities/Transaction.h"

namespace ui {

struct TransactionDraft {
    QString name;
    QString bookingDate;
    QString valuta;
    double amount = 0.0;

    QString actorText;
    QString propertyText;
    QString actorId;
    bool actorSelected = false;
    QString contractId;
    bool contractSelected = false;

    QString metadata;
    QString proofImageData;

    QString type;

    ImportDraftSuggestions suggestions;

    bool allocatable = false;
    bool allocatableSelected = false;

    // Default to Unverified so new transactions require user verification
    int status = static_cast<int>(core::domain::Transaction::Status::Unverified);

    // Associated property ids (UI-level)
    QStringList propertyIds;
};

}
