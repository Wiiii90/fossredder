#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/ActorList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/ContractList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/AnalysisList.h"
#include "ui/models/AnnualList.h"
#include "ui/state/SelectionState.h"
#include "ui/state/FilterState.h"
#include "ui/state/MetricsState.h"

namespace ui {

class StateFacade : public QObject {
    Q_OBJECT
    Q_PROPERTY(ActorList* actors READ actors CONSTANT)
    Q_PROPERTY(PropertyList* properties READ properties CONSTANT)
    Q_PROPERTY(ContractList* contracts READ contracts CONSTANT)
    Q_PROPERTY(StatementList* statements READ statements CONSTANT)
    Q_PROPERTY(TransactionList* transactions READ transactions CONSTANT)
    Q_PROPERTY(AnalysisList* analyses READ analyses CONSTANT)
    Q_PROPERTY(AnnualList* annuals READ annuals CONSTANT)

    Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE setSelectedActorId NOTIFY selectedActorIdChanged)
    Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
    Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE setSelectedContractId NOTIFY selectedContractIdChanged)
    Q_PROPERTY(QString selectedStatementId READ selectedStatementId WRITE setSelectedStatementId NOTIFY selectedStatementIdChanged)
    Q_PROPERTY(QString selectedTransactionId READ selectedTransactionId WRITE setSelectedTransactionId NOTIFY selectedTransactionIdChanged)
    Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId WRITE setSelectedAnalysisId NOTIFY selectedAnalysisIdChanged)
    Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId WRITE setSelectedAnnualId NOTIFY selectedAnnualIdChanged)

    Q_PROPERTY(EntitySelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(EntitySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(EntitySelection* selectedContract READ selectedContract CONSTANT)
    Q_PROPERTY(EntitySelection* selectedStatement READ selectedStatement CONSTANT)
    Q_PROPERTY(EntitySelection* selectedTransaction READ selectedTransaction CONSTANT)
    Q_PROPERTY(EntitySelection* selectedAnalysis READ selectedAnalysis CONSTANT)
    Q_PROPERTY(EntitySelection* selectedAnnual READ selectedAnnual CONSTANT)

    Q_PROPERTY(QVariant lastAnalysisResult READ lastAnalysisResult WRITE setLastAnalysisResult NOTIFY lastAnalysisResultChanged)

public:
    explicit StateFacade(QObject* parent = nullptr);

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

    void setSelectedActorId(const QString& id);
    void setSelectedPropertyId(const QString& id);
    void setSelectedContractId(const QString& id);
    void setSelectedStatementId(const QString& id);
    void setSelectedTransactionId(const QString& id);
    void setSelectedAnalysisId(const QString& id);
    void setSelectedAnnualId(const QString& id);

    EntitySelection* selectedActor();
    EntitySelection* selectedProperty();
    EntitySelection* selectedContract();
    EntitySelection* selectedStatement();
    EntitySelection* selectedTransaction();
    EntitySelection* selectedAnalysis();
    EntitySelection* selectedAnnual();

    Q_INVOKABLE QVariantList statementTransactionIds(const QString& statementId) const;
    Q_INVOKABLE QObject* statementTransactions(const QString& statementId);
    Q_INVOKABLE QObject* propertyTransactions(const QString& propertyId);
    Q_INVOKABLE QStringList propertyContractTypes(const QString& propertyId) const;
    Q_INVOKABLE QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    Q_INVOKABLE QString propertyName(const QString& id) const;

    void applyTransactionUpdates(const std::vector<std::string>& ids, const AppState& state);

    Q_INVOKABLE void applyDeletionImpact(const DeletionImpact& impact);
    Q_INVOKABLE void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

    QVariant lastAnalysisResult() const { return lastAnalysisResult_; }
    void setLastAnalysisResult(const QVariant& v) { lastAnalysisResult_ = v; emit lastAnalysisResultChanged(); }

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();
    void selectedStatementIdChanged();
    void selectedTransactionIdChanged();
    void selectedAnalysisIdChanged();
    void selectedAnnualIdChanged();
    void transactionSumsUpdated(const QString& propertyId);
    void lastAnalysisResultChanged();

private:
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

    QVariant lastAnalysisResult_;
};

}
