/**
 * @file ui/src/state/SessionSelection.cpp
 * @brief Implements synchronized UI selection tracking across session models.
 */

#include "ui/state/session/WorkspaceSessionSelection.h"

#include <QAbstractItemModel>

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

void emitSelectionChanges(SessionSelection& selection, const SelectionIdsSnapshot& before)
{
    if (selection.selectedActorId() != before.actorId) emit selection.selectedActorIdChanged();
    if (selection.selectedPropertyId() != before.propertyId) emit selection.selectedPropertyIdChanged();
    if (selection.selectedContractId() != before.contractId) emit selection.selectedContractIdChanged();
    if (selection.selectedStatementId() != before.statementId) emit selection.selectedStatementIdChanged();
    if (selection.selectedTransactionId() != before.transactionId) emit selection.selectedTransactionIdChanged();
    if (selection.selectedAnalysisId() != before.analysisId) emit selection.selectedAnalysisIdChanged();
    if (selection.selectedAnnualId() != before.annualId) emit selection.selectedAnnualIdChanged();
}

template <typename RefreshFn>
void bindSelectionRefreshesForModel(QObject* owner, RefreshFn&& refresh, QAbstractItemModel& model)
{
    QObject::connect(&model, &QAbstractItemModel::modelReset, owner, refresh);
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, owner, refresh);
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, owner, refresh);
    QObject::connect(&model, &QAbstractItemModel::rowsMoved, owner, refresh);
    QObject::connect(&model, &QAbstractItemModel::layoutChanged, owner, refresh);
    QObject::connect(&model, &QAbstractItemModel::dataChanged, owner,
                     [refresh](const QModelIndex&, const QModelIndex&, const QVector<int>&) {
                         refresh();
                     });
}

} // namespace

SessionSelection::SessionSelection(SessionModels& models, QObject* parent)
    : QObject(parent)
    , models_(models)
    , state_(models_.actors(),
             models_.properties(),
             models_.contracts(),
             models_.statements(),
             models_.transactions(),
             models_.analyses(),
             models_.annuals(),
             this)
{
    bindModelSignals();
}

void SessionSelection::loadFromState()
{
    refreshSelections();
    clearAnalysisResult();
}

QString SessionSelection::selectedActorId() const { return state_.selectedActorId(); }
QString SessionSelection::selectedPropertyId() const { return state_.selectedPropertyId(); }
QString SessionSelection::selectedContractId() const { return state_.selectedContractId(); }
QString SessionSelection::selectedStatementId() const { return state_.selectedStatementId(); }
QString SessionSelection::selectedTransactionId() const { return state_.selectedTransactionId(); }
QString SessionSelection::selectedAnalysisId() const { return state_.selectedAnalysisId(); }
QString SessionSelection::selectedAnnualId() const { return state_.selectedAnnualId(); }

void SessionSelection::setSelectedActorId(const QString& id)
{
    if (state_.setSelectedActorId(id)) emit selectedActorIdChanged();
}

void SessionSelection::setSelectedPropertyId(const QString& id)
{
    if (state_.setSelectedPropertyId(id)) emit selectedPropertyIdChanged();
}

void SessionSelection::setSelectedContractId(const QString& id)
{
    if (state_.setSelectedContractId(id)) emit selectedContractIdChanged();
}

void SessionSelection::setSelectedStatementId(const QString& id)
{
    if (state_.setSelectedStatementId(id)) emit selectedStatementIdChanged();
}

void SessionSelection::setSelectedTransactionId(const QString& id)
{
    if (state_.setSelectedTransactionId(id)) emit selectedTransactionIdChanged();
}

void SessionSelection::setSelectedAnalysisId(const QString& id)
{
    if (!state_.setSelectedAnalysisId(id)) return;
    clearAnalysisResult();
    emit selectedAnalysisIdChanged();
}

void SessionSelection::setSelectedAnnualId(const QString& id)
{
    if (state_.setSelectedAnnualId(id)) emit selectedAnnualIdChanged();
}

ActorSelection* SessionSelection::selectedActor() { return state_.selectedActor(); }
PropertySelection* SessionSelection::selectedProperty() { return state_.selectedProperty(); }
ContractSelection* SessionSelection::selectedContract() { return state_.selectedContract(); }
StatementSelection* SessionSelection::selectedStatement() { return state_.selectedStatement(); }
TransactionSelection* SessionSelection::selectedTransaction() { return state_.selectedTransaction(); }
AnalysisSelection* SessionSelection::selectedAnalysis() { return state_.selectedAnalysis(); }
AnnualSelection* SessionSelection::selectedAnnual() { return state_.selectedAnnual(); }

void SessionSelection::setLastAnalysisResult(const QVariant& value)
{
    if (lastAnalysisResult_ == value) return;
    lastAnalysisResult_ = value;
    emit lastAnalysisResultChanged();
}

void SessionSelection::bindModelSignals()
{
    auto refresh = [this]() {
        refreshSelections();
    };
    bindSelectionRefreshesForModel(this, refresh, models_.actors());
    bindSelectionRefreshesForModel(this, refresh, models_.properties());
    bindSelectionRefreshesForModel(this, refresh, models_.contracts());
    bindSelectionRefreshesForModel(this, refresh, models_.statements());
    bindSelectionRefreshesForModel(this, refresh, models_.transactions());
    bindSelectionRefreshesForModel(this, refresh, models_.analyses());
    bindSelectionRefreshesForModel(this, refresh, models_.annuals());
}

void SessionSelection::refreshSelections()
{
    const auto before = captureSelectionIds(state_);
    state_.refreshAll();
    if (selectedAnalysisId() != before.analysisId) clearAnalysisResult();
    emitSelectionChanges(*this, before);
}

void SessionSelection::clearAnalysisResult()
{
    if (!lastAnalysisResult_.isValid()) return;
    lastAnalysisResult_.clear();
    emit lastAnalysisResultChanged();
}

}
