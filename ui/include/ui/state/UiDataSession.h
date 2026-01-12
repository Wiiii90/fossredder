#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <QSet>
#include <QVariantMap>

#include "core/models/AppState.h"
#include "ui/models/ActorList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/ContractList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/TransactionFilterModel.h"
#include "ui/state/UiEntitySelection.h"
#include "core/models/DeletionImpact.h"

class UiDataSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(ActorList* actors READ actors CONSTANT)
    Q_PROPERTY(PropertyList* properties READ properties CONSTANT)
    Q_PROPERTY(ContractList* contracts READ contracts CONSTANT)
    Q_PROPERTY(StatementList* statements READ statements CONSTANT)
    Q_PROPERTY(TransactionList* transactions READ transactions CONSTANT)

    Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE setSelectedActorId NOTIFY selectedActorIdChanged)
    Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
    Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE setSelectedContractId NOTIFY selectedContractIdChanged)

    Q_PROPERTY(QString selectedStatementId READ selectedStatementId WRITE setSelectedStatementId NOTIFY selectedStatementIdChanged)
    Q_PROPERTY(QString selectedTransactionId READ selectedTransactionId WRITE setSelectedTransactionId NOTIFY selectedTransactionIdChanged)

    Q_PROPERTY(UiEntitySelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedContract READ selectedContract CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedStatement READ selectedStatement CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedTransaction READ selectedTransaction CONSTANT)

public:
    explicit UiDataSession(QObject* parent = nullptr);

    ActorList* actors() noexcept { return &actors_; }
    PropertyList* properties() noexcept { return &properties_; }
    ContractList* contracts() noexcept { return &contracts_; }
    StatementList* statements() noexcept { return &statements_; }
    TransactionList* transactions() noexcept { return &transactions_; }

    void loadFromState(const AppState& state);

    QString selectedActorId() const { return selectedActorId_; }
    QString selectedPropertyId() const { return selectedPropertyId_; }
    QString selectedContractId() const { return selectedContractId_; }
    QString selectedStatementId() const { return selectedStatementId_; }
    QString selectedTransactionId() const { return selectedTransactionId_; }

    void setSelectedActorId(const QString& id);
    void setSelectedPropertyId(const QString& id);
    void setSelectedContractId(const QString& id);
    void setSelectedStatementId(const QString& id);
    void setSelectedTransactionId(const QString& id);

    UiEntitySelection* selectedActor() noexcept { return &selectedActor_; }
    UiEntitySelection* selectedProperty() noexcept { return &selectedProperty_; }
    UiEntitySelection* selectedContract() noexcept { return &selectedContract_; }
    UiEntitySelection* selectedStatement() noexcept { return &selectedStatement_; }
    UiEntitySelection* selectedTransaction() noexcept { return &selectedTransaction_; }

    Q_INVOKABLE QVariantList transactionIdsForStatement(const QString& statementId) const;
    Q_INVOKABLE QObject* transactionsForStatement(const QString& statementId);
    Q_INVOKABLE QObject* transactionsForProperty(const QString& propertyId);
    Q_INVOKABLE QStringList transactionTypesForProperty(const QString& propertyId) const;
    Q_INVOKABLE QVariantMap transactionSumsForPropertyWithType(const QString& propertyId, const QString& type) const;
    Q_INVOKABLE QVariantMap transactionSumsForProperty(const QString& propertyId) const;

    // Apply incremental updates for specific transaction ids using current AppState
    void applyTransactionUpdates(const std::vector<std::string>& ids, const AppState& state);

    Q_INVOKABLE void applyDeletionImpact(const DeletionImpact& impact);

    Q_INVOKABLE void setEditingTransaction(const QString& txId, bool editing);
    Q_INVOKABLE bool isEditingTransaction(const QString& txId) const;

    // Update transaction's property ids immediately in the UI model (no persistence)
    Q_INVOKABLE void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();
    void selectedStatementIdChanged();
    void selectedTransactionIdChanged();

    // Emitted when sums for a specific property changed
    void transactionSumsUpdated(const QString& propertyId);

private:
    ActorList actors_;
    PropertyList properties_;
    ContractList contracts_;
    StatementList statements_;
    TransactionList transactions_;

    QString selectedActorId_;
    QString selectedPropertyId_;
    QString selectedContractId_;
    QString selectedStatementId_;
    QString selectedTransactionId_;

    UiEntitySelection selectedActor_;
    UiEntitySelection selectedProperty_;
    UiEntitySelection selectedContract_;
    UiEntitySelection selectedStatement_;
    UiEntitySelection selectedTransaction_;

    mutable QHash<QString, TransactionFilterModel*> txByStatement_;
    mutable QHash<QString, TransactionFilterModel*> txByProperty_;

    // Cache of computed sums per property id
    mutable QHash<QString, QVariantMap> propertySumsCache_;

    // temporary set to track property ids from rows about to be removed
    mutable QSet<QString> pendingRecomputePropertyIds_;

    // transactions currently being edited in the UI (suppress incremental overwrite)
    QSet<QString> editingTransactions_;
    // transactions pending deletion while user edits them
    QSet<QString> pendingDeletionTransactions_;

    void refreshSelectedActor();
    void refreshSelectedProperty();
    void refreshSelectedContract();
    void refreshSelectedStatement();
    void refreshSelectedTransaction();

    // recompute helpers
    void recomputeAllPropertySums();
    void recomputePropertySum(const QString& propertyId) const;
};
