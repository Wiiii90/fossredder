#pragma once

#include <QString>

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

    int status = 0;
};
