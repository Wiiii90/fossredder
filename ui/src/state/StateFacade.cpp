#include "ui/state/StateFacade.h"

namespace ui {

namespace {

struct SelectionIdsSnapshot {
    QString actorId;
    QString propertyId;
    QString contractId;
    QString statementId;
    QString transactionId;
    QString analysisId;
    QString annualId;
};

SelectionIdsSnapshot captureSelectionIds(const SelectionState& selection)
{
    return {
        selection.selectedActorId(),
        selection.selectedPropertyId(),
        selection.selectedContractId(),
        selection.selectedStatementId(),
        selection.selectedTransactionId(),
        selection.selectedAnalysisId(),
        selection.selectedAnnualId()
    };
}

void emitSelectionChanges(StateFacade& facade, const SelectionIdsSnapshot& before, const SelectionState& selection)
{
    if (selection.selectedActorId() != before.actorId) emit facade.selectedActorIdChanged();
    if (selection.selectedPropertyId() != before.propertyId) emit facade.selectedPropertyIdChanged();
    if (selection.selectedContractId() != before.contractId) emit facade.selectedContractIdChanged();
    if (selection.selectedStatementId() != before.statementId) emit facade.selectedStatementIdChanged();
    if (selection.selectedTransactionId() != before.transactionId) emit facade.selectedTransactionIdChanged();
    if (selection.selectedAnalysisId() != before.analysisId) emit facade.selectedAnalysisIdChanged();
    if (selection.selectedAnnualId() != before.annualId) emit facade.selectedAnnualIdChanged();
}

}

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , store_(this)
    , selection_(store_.models().actors(),
                 store_.models().properties(),
                 store_.models().contracts(),
                 store_.models().statements(),
                 store_.models().transactions(),
                 store_.models().analyses(),
                 store_.models().annuals(),
                 this)
{
    QObject::connect(&store_, &SessionStore::selectionRefreshRequested, this, [this]() {
        const auto before = captureSelectionIds(selection_);
        selection_.refreshAll();
        emitSelectionChanges(*this, before, selection_);
    });
    QObject::connect(&store_, &SessionStore::transactionSumsUpdated, this, &StateFacade::transactionSumsUpdated);
}

void StateFacade::loadFromState(const AppState& state)
{
    store_.loadFromState(state);
    const QVariant previousAnalysisResult = lastAnalysisResult_;
    lastAnalysisResult_.clear();
    if (lastAnalysisResult_ != previousAnalysisResult) emit lastAnalysisResultChanged();
}

ActorList* StateFacade::actors() noexcept { return &store_.models().actors(); }
PropertyList* StateFacade::properties() noexcept { return &store_.models().properties(); }
ContractList* StateFacade::contracts() noexcept { return &store_.models().contracts(); }
StatementList* StateFacade::statements() noexcept { return &store_.models().statements(); }
TransactionList* StateFacade::transactions() noexcept { return &store_.models().transactions(); }
AnalysisList* StateFacade::analyses() noexcept { return &store_.models().analyses(); }
AnnualList* StateFacade::annuals() noexcept { return &store_.models().annuals(); }

QString StateFacade::selectedActorId() const { return selection_.selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return selection_.selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return selection_.selectedContractId(); }
QString StateFacade::selectedStatementId() const { return selection_.selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return selection_.selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return selection_.selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return selection_.selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    if (selection_.setSelectedActorId(id)) emit selectedActorIdChanged();
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    if (selection_.setSelectedPropertyId(id)) emit selectedPropertyIdChanged();
}

void StateFacade::setSelectedContractId(const QString& id)
{
    if (selection_.setSelectedContractId(id)) emit selectedContractIdChanged();
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    if (selection_.setSelectedStatementId(id)) emit selectedStatementIdChanged();
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    if (selection_.setSelectedTransactionId(id)) emit selectedTransactionIdChanged();
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    if (!selection_.setSelectedAnalysisId(id)) return;

    if (lastAnalysisResult_.isValid()) {
        lastAnalysisResult_.clear();
        emit lastAnalysisResultChanged();
    }

    emit selectedAnalysisIdChanged();
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    if (selection_.setSelectedAnnualId(id)) emit selectedAnnualIdChanged();
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
    for (const auto& transaction : store_.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) out.push_back(QString::fromStdString(transaction->id));
    }
    return out;
}

TransactionFilter* StateFacade::statementTransactions(const QString& statementId)
{
    return store_.statementTransactions(statementId, this);
}

TransactionFilter* StateFacade::propertyTransactions(const QString& propertyId)
{
    return store_.propertyTransactions(propertyId, this);
}

QStringList StateFacade::propertyContractTypes(const QString& propertyId) const
{
    return store_.propertyContractTypes(propertyId);
}

QVariantMap StateFacade::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return store_.propertyTransactionSums(propertyId, contractType);
}

QString StateFacade::propertyName(const QString& id) const
{
    return store_.propertyName(id);
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    store_.applyDeletionImpact(impact);
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    store_.setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}
