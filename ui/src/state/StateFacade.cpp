/**
 * @file ui/src/state/StateFacade.cpp
 * @brief Implements the UI session facade that combines session data and selection state.
 */

#include "ui/state/StateFacade.h"

namespace ui {

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , session_(this)
    , selection_(session_.models(), this)
{
    QObject::connect(&session_, &SessionStore::transactionSumsUpdated, this, &StateFacade::transactionSumsUpdated);
    QObject::connect(&selection_, &SessionSelection::selectedActorIdChanged, this, &StateFacade::selectedActorIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedPropertyIdChanged, this, &StateFacade::selectedPropertyIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedContractIdChanged, this, &StateFacade::selectedContractIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedStatementIdChanged, this, &StateFacade::selectedStatementIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedTransactionIdChanged, this, &StateFacade::selectedTransactionIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedAnalysisIdChanged, this, &StateFacade::selectedAnalysisIdChanged);
    QObject::connect(&selection_, &SessionSelection::selectedAnnualIdChanged, this, &StateFacade::selectedAnnualIdChanged);
    QObject::connect(&selection_, &SessionSelection::lastAnalysisResultChanged, this, &StateFacade::lastAnalysisResultChanged);
}

SessionStore* StateFacade::session() noexcept { return &session_; }
SessionSelection* StateFacade::selection() noexcept { return &selection_; }

void StateFacade::loadFromState(const AppState& state)
{
    session_.loadFromState(state);
    selection_.loadFromState();
}

ActorList* StateFacade::actors() noexcept { return &session_.models().actors(); }
PropertyList* StateFacade::properties() noexcept { return &session_.models().properties(); }
ContractList* StateFacade::contracts() noexcept { return &session_.models().contracts(); }
StatementList* StateFacade::statements() noexcept { return &session_.models().statements(); }
TransactionList* StateFacade::transactions() noexcept { return &session_.models().transactions(); }
AnalysisList* StateFacade::analyses() noexcept { return &session_.models().analyses(); }
AnnualList* StateFacade::annuals() noexcept { return &session_.models().annuals(); }

QString StateFacade::selectedActorId() const { return selection_.selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return selection_.selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return selection_.selectedContractId(); }
QString StateFacade::selectedStatementId() const { return selection_.selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return selection_.selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return selection_.selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return selection_.selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    selection_.setSelectedActorId(id);
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    selection_.setSelectedPropertyId(id);
}

void StateFacade::setSelectedContractId(const QString& id)
{
    selection_.setSelectedContractId(id);
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    selection_.setSelectedStatementId(id);
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    selection_.setSelectedTransactionId(id);
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    selection_.setSelectedAnalysisId(id);
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    selection_.setSelectedAnnualId(id);
}

ActorSelection* StateFacade::selectedActor() { return selection_.selectedActor(); }
PropertySelection* StateFacade::selectedProperty() { return selection_.selectedProperty(); }
ContractSelection* StateFacade::selectedContract() { return selection_.selectedContract(); }
StatementSelection* StateFacade::selectedStatement() { return selection_.selectedStatement(); }
TransactionSelection* StateFacade::selectedTransaction() { return selection_.selectedTransaction(); }
AnalysisSelection* StateFacade::selectedAnalysis() { return selection_.selectedAnalysis(); }
AnnualSelection* StateFacade::selectedAnnual() { return selection_.selectedAnnual(); }

QVariantList StateFacade::statementTransactionIds(const QString& statementId) const
{
    QVariantList out;
    if (statementId.isEmpty()) return out;
    for (const auto& transaction : session_.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) out.push_back(QString::fromStdString(transaction->id));
    }
    return out;
}

TransactionFilter* StateFacade::statementTransactions(const QString& statementId)
{
    return session_.statementTransactions(statementId, this);
}

TransactionFilter* StateFacade::propertyTransactions(const QString& propertyId)
{
    return session_.propertyTransactions(propertyId, this);
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
    session_.applyDeletionImpact(impact);
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_.setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}
