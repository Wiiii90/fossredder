#pragma once

#include <functional>

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/ActorList.h"
#include "ui/models/AnalysisList.h"
#include "ui/models/AnnualList.h"
#include "ui/models/ContractList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"
#include "ui/state/FilterState.h"
#include "ui/state/MetricsState.h"
#include "ui/state/SelectionState.h"

namespace ui {

class SessionStore {
public:
    using TransactionSumsNotifier = std::function<void(const QString& propertyId)>;

    explicit SessionStore(QObject* owner, TransactionSumsNotifier notifier = {});

    ActorList* actors() noexcept { return &actors_; }
    PropertyList* properties() noexcept { return &properties_; }
    ContractList* contracts() noexcept { return &contracts_; }
    StatementList* statements() noexcept { return &statements_; }
    TransactionList* transactions() noexcept { return &transactions_; }
    AnalysisList* analyses() noexcept { return &analyses_; }
    AnnualList* annuals() noexcept { return &annuals_; }

    void loadFromState(const AppState& state);

    QString selectedActorId() const;
    QString selectedPropertyId() const;
    QString selectedContractId() const;
    QString selectedStatementId() const;
    QString selectedTransactionId() const;
    QString selectedAnalysisId() const;
    QString selectedAnnualId() const;

    bool setSelectedActorId(const QString& id);
    bool setSelectedPropertyId(const QString& id);
    bool setSelectedContractId(const QString& id);
    bool setSelectedStatementId(const QString& id);
    bool setSelectedTransactionId(const QString& id);
    bool setSelectedAnalysisId(const QString& id);
    bool setSelectedAnnualId(const QString& id);

    ActorSelection* selectedActor();
    PropertySelection* selectedProperty();
    ContractSelection* selectedContract();
    StatementSelection* selectedStatement();
    TransactionSelection* selectedTransaction();
    AnalysisSelection* selectedAnalysis();
    AnnualSelection* selectedAnnual();

    QVariantList statementTransactionIds(const QString& statementId) const;
    QObject* statementTransactions(const QString& statementId);
    QObject* propertyTransactions(const QString& propertyId);
    QStringList propertyContractTypes(const QString& propertyId) const;
    QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    QString propertyName(const QString& id) const;

    void applyDeletionImpact(const DeletionImpact& impact);
    void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

private:
    void bindModelSignals();
    void rebuildPropertyNameIndex();
    void recomputeAllMetrics();
    void recomputeMetricsForRows(int firstRow, int lastRow);
    void recomputeMetricsForPropertyIds(const QSet<QString>& propertyIds);
    void notifyTransactionSumsForAllProperties() const;

    QObject* owner_ = nullptr;
    TransactionSumsNotifier transactionSumsNotifier_;
    ActorList actors_;
    AnalysisList analyses_;
    PropertyList properties_;
    ContractList contracts_;
    StatementList statements_;
    TransactionList transactions_;
    AnnualList annuals_;
    SelectionState selection_;
    FilterState filters_;
    mutable MetricsState metrics_;
    QHash<QString, QString> propertyNameIndex_;
};

}
