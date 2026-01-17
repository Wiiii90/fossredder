#pragma once

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QString>

#include "core/controllers/AppStateController.h"

class StatementDraft;

class UiDomainController : public QObject {
    Q_OBJECT
public:
    explicit UiDomainController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE QString addActorWithAliases(const QString& name, const QString& type, const QString& description, const QStringList& aliases);
    Q_INVOKABLE QStringList getActorAliases(const QString& actorId) const;
    Q_INVOKABLE void setActorAliases(const QString& actorId, const QStringList& aliases);

    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description);

    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                   const QStringList& actorIds, const QStringList& propertyIds);

    Q_INVOKABLE QString addStatement(const QString& name);
    Q_INVOKABLE QString addTransaction(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId);
    Q_INVOKABLE QString addTransactionWithStatus(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status);
    Q_INVOKABLE QString addTransactionDetailed(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status, const QString& actorId);

    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);

    Q_INVOKABLE bool tryAutoAssignActorForDraftTransaction(StatementDraft* draft, int index);
    Q_INVOKABLE int autoAssignActorsForDraft(StatementDraft* draft);
    Q_INVOKABLE bool canFinalizeStatementDraft(StatementDraft* draft) const;

    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void updateActorWithAliases(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases);
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description);

    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                   const QStringList& actorIds, const QStringList& propertyIds);

    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);
    Q_INVOKABLE void updateTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId);
    Q_INVOKABLE void updateTransactionStatus(const QString& id, int status);
    Q_INVOKABLE void updateTransactionActor(const QString& id, const QString& actorId);
    Q_INVOKABLE void updateTransactionAllocatable(const QString& id, bool allocatable);
    Q_INVOKABLE void updateTransactionProperties(const QString& id, const QStringList& propertyIds);
    Q_INVOKABLE void exportData(const QString& format, bool includeFormulas, const QString& path);
    

    Q_INVOKABLE void deleteActor(const QString& id);
    Q_INVOKABLE void deleteProperty(const QString& id);
    Q_INVOKABLE void deleteContract(const QString& id);
    Q_INVOKABLE void deleteStatement(const QString& id);
    Q_INVOKABLE void deleteTransaction(const QString& id);

    Q_INVOKABLE QString ensureActorByName(const QString& name);
    Q_INVOKABLE QString ensurePropertyByName(const QString& name);
    Q_INVOKABLE QString ensureStatementByName(const QString& name);

private:
    AppStateController* core_;

    void pruneInvalidContracts();
    void pruneInvalidTransactions();

    // Debounce timers keyed by an arbitrary string (e.g. "tx:<id>") used to
    // coalesce rapid updates into a single commit.
    QHash<QString, QTimer*> commitTimers_;

    // Schedule a debounced commit; subsequent calls with same key within
    // `ms` milliseconds will restart the timer.
    void scheduleDebouncedCommit(const QString& key, int ms = 300);

    // (Contracts are created explicitly when draft transaction has a type.)
};
