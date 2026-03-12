#pragma once

#include <QString>
#include <QStringList>
#include "core/models/Transaction.h"

namespace ui {

struct TransactionDraft {
    QString name;
    QString bookingDate;
    QString valuta;
    double amount = 0.0;

    QString description;

    QString actorId;

    QString metadata;
    QString proofImagePath;

    QString type;

    bool allocatable = false;

    // Default to Unverified so new transactions require user verification
    int status = static_cast<int>(core::domain::Transaction::Status::Unverified);

    // Associated property ids (UI-level)
    QStringList propertyIds;
};

}
