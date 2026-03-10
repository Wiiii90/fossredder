#pragma once

#include <QObject>
#include <QVariant>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/TransactionFilter.h"
#include "ui/state/SelectionState.h"
#include "ui/state/SessionStore.h"

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

    Q_PROPERTY(ActorSelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(PropertySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(ContractSelection* selectedContract READ selectedContract CONSTANT)
    Q_PROPERTY(StatementSelection* selectedStatement READ selectedStatement CONSTANT)
    Q_PROPERTY(TransactionSelection* selectedTransaction READ selectedTransaction CONSTANT)
    Q_PROPERTY(AnalysisSelection* selectedAnalysis READ selectedAnalysis CONSTANT)
    Q_PROPERTY(AnnualSelection* selectedAnnual READ selectedAnnual CONSTANT)

    Q_PROPERTY(QVariant lastAnalysisResult READ lastAnalysisResult WRITE setLastAnalysisResult NOTIFY lastAnalysisResultChanged)

public:
    explicit StateFacade(QObject* parent = nullptr);

    ActorList* actors() noexcept;
    PropertyList* properties() noexcept;
    ContractList* contracts() noexcept;
    StatementList* statements() noexcept;
    TransactionList* transactions() noexcept;
    AnalysisList* analyses() noexcept;
    AnnualList* annuals() noexcept;

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

    ActorSelection* selectedActor();
    PropertySelection* selectedProperty();
    ContractSelection* selectedContract();
    StatementSelection* selectedStatement();
    TransactionSelection* selectedTransaction();
    AnalysisSelection* selectedAnalysis();
    AnnualSelection* selectedAnnual();

    Q_INVOKABLE QVariantList statementTransactionIds(const QString& statementId) const;
    Q_INVOKABLE TransactionFilter* statementTransactions(const QString& statementId);
    Q_INVOKABLE TransactionFilter* propertyTransactions(const QString& propertyId);
    Q_INVOKABLE QStringList propertyContractTypes(const QString& propertyId) const;
    Q_INVOKABLE QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    Q_INVOKABLE QString propertyName(const QString& id) const;

    Q_INVOKABLE void applyDeletionImpact(const DeletionImpact& impact);
    Q_INVOKABLE void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

    QVariant lastAnalysisResult() const { return lastAnalysisResult_; }
    void setLastAnalysisResult(const QVariant& v) { if (lastAnalysisResult_ == v) return; lastAnalysisResult_ = v; emit lastAnalysisResultChanged(); }

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
    SessionStore store_;
    SelectionState selection_;

    QVariant lastAnalysisResult_;
};

}
