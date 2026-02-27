#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/controllers/AppStateController.h"
#include "core/models/Transaction.h"

namespace ui {

class TransactionController : public QObject {
    Q_OBJECT
public:
    explicit TransactionController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addTransaction(const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status = static_cast<int>(Transaction::Status::Neutral),
                                       const QString& actorId = QString(),
                                       bool allocatable = false,
                                       const QStringList& propertyIds = {});

    Q_INVOKABLE void updateTransaction(const QString& id,
                                       const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status,
                                       const QString& actorId,
                                       bool allocatable,
                                       const QStringList& propertyIds);

    Q_INVOKABLE void deleteTransaction(const QString& id);

private:
    AppStateController* core_ = nullptr;
};

}
