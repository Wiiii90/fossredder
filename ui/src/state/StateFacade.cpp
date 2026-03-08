#include "ui/state/StateFacade.h"

namespace ui {

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , session_(this, [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    })
{
}

void StateFacade::loadFromState(const AppState& state)
{
    session_.loadFromState(state);
    const QVariant previousAnalysisResult = lastAnalysisResult_;
    lastAnalysisResult_.clear();
    if (lastAnalysisResult_ != previousAnalysisResult) emit lastAnalysisResultChanged();
}

ActorList* StateFacade::actors() noexcept { return session_.actors(); }
PropertyList* StateFacade::properties() noexcept { return session_.properties(); }
ContractList* StateFacade::contracts() noexcept { return session_.contracts(); }
StatementList* StateFacade::statements() noexcept { return session_.statements(); }
TransactionList* StateFacade::transactions() noexcept { return session_.transactions(); }
AnalysisList* StateFacade::analyses() noexcept { return session_.analyses(); }
AnnualList* StateFacade::annuals() noexcept { return session_.annuals(); }

QString StateFacade::selectedActorId() const { return session_.selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return session_.selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return session_.selectedContractId(); }
QString StateFacade::selectedStatementId() const { return session_.selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return session_.selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return session_.selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return session_.selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    if (session_.setSelectedActorId(id)) emit selectedActorIdChanged();
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    if (session_.setSelectedPropertyId(id)) emit selectedPropertyIdChanged();
}

void StateFacade::setSelectedContractId(const QString& id)
{
    if (session_.setSelectedContractId(id)) emit selectedContractIdChanged();
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    if (session_.setSelectedStatementId(id)) emit selectedStatementIdChanged();
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    if (session_.setSelectedTransactionId(id)) emit selectedTransactionIdChanged();
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    if (session_.setSelectedAnalysisId(id)) emit selectedAnalysisIdChanged();
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    if (session_.setSelectedAnnualId(id)) emit selectedAnnualIdChanged();
}

ActorSelection* StateFacade::selectedActor() { return session_.selectedActor(); }
PropertySelection* StateFacade::selectedProperty() { return session_.selectedProperty(); }
ContractSelection* StateFacade::selectedContract() { return session_.selectedContract(); }
StatementSelection* StateFacade::selectedStatement() { return session_.selectedStatement(); }
TransactionSelection* StateFacade::selectedTransaction() { return session_.selectedTransaction(); }
AnalysisSelection* StateFacade::selectedAnalysis() { return session_.selectedAnalysis(); }
AnnualSelection* StateFacade::selectedAnnual() { return session_.selectedAnnual(); }

QVariantList StateFacade::statementTransactionIds(const QString& statementId) const
{
    return session_.statementTransactionIds(statementId);
}

QObject* StateFacade::statementTransactions(const QString& statementId)
{
    return session_.statementTransactions(statementId);
}

QObject* StateFacade::propertyTransactions(const QString& propertyId)
{
    return session_.propertyTransactions(propertyId);
}

QStringList StateFacade::propertyContractTypes(const QString& propertyId) const
{
    return session_.propertyContractTypes(propertyId);
}

QVariantMap StateFacade::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return session_.propertyTransactionSums(propertyId, contractType);
}

QString StateFacade::propertyName(const QString& id) const
{
    return session_.propertyName(id);
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    const QString previousActorId = session_.selectedActorId();
    const QString previousPropertyId = session_.selectedPropertyId();
    const QString previousContractId = session_.selectedContractId();
    const QString previousStatementId = session_.selectedStatementId();
    const QString previousTransactionId = session_.selectedTransactionId();

    session_.applyDeletionImpact(impact);

    if (session_.selectedActorId() != previousActorId) emit selectedActorIdChanged();
    if (session_.selectedPropertyId() != previousPropertyId) emit selectedPropertyIdChanged();
    if (session_.selectedContractId() != previousContractId) emit selectedContractIdChanged();
    if (session_.selectedStatementId() != previousStatementId) emit selectedStatementIdChanged();
    if (session_.selectedTransactionId() != previousTransactionId) emit selectedTransactionIdChanged();
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_.setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}
