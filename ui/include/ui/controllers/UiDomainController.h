#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/FileController.h"

class UiDomainController : public QObject {
    Q_OBJECT
public:
    explicit UiDomainController(FileController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description);

    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                   const QStringList& actorIds, const QStringList& propertyIds);

    Q_INVOKABLE QString addStatement(const QString& name, const QString& startDate, const QString& endDate);
    Q_INVOKABLE QString addTransaction(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId);
    Q_INVOKABLE QString addTransactionWithStatus(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status);

    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description);

    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                   const QStringList& actorIds, const QStringList& propertyIds);

    Q_INVOKABLE void updateStatement(const QString& id, const QString& name, const QString& startDate, const QString& endDate);
    Q_INVOKABLE void updateTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId);
    Q_INVOKABLE void updateTransactionStatus(const QString& id, int status);

    Q_INVOKABLE void deleteActor(const QString& id);
    Q_INVOKABLE void deleteProperty(const QString& id);
    Q_INVOKABLE void deleteContract(const QString& id);
    Q_INVOKABLE void deleteStatement(const QString& id);
    Q_INVOKABLE void deleteTransaction(const QString& id);

    Q_INVOKABLE QString ensureActorByName(const QString& name);
    Q_INVOKABLE QString ensurePropertyByName(const QString& name);
    Q_INVOKABLE QString ensureStatementByName(const QString& name);

private:
    FileController* core_;

    void pruneInvalidContracts();
    void pruneInvalidTransactions();
};
