/**
 * @file ui/src/state/SessionSelection.cpp
 * @brief Implements synchronized UI selection tracking across session models.
 */

#include "ui/state/SessionSelection.h"

#include <QAbstractItemModel>

#include <utility>

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

/** @brief Captures the currently selected ids before a refresh cycle. */
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

/** @brief Emits only the selection changed signals that actually changed. */
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

/** @brief Binds selection refresh logic to relevant model change signals. */
template <typename RefreshFn>
void bindSelectionRefresh(QAbstractItemModel& model, QObject* context, RefreshFn&& refresh)
{
    auto trigger = [refresh = std::forward<RefreshFn>(refresh)]() mutable {
        refresh();
    };

    QObject::connect(&model, &QAbstractItemModel::dataChanged, context, [trigger](const QModelIndex&, const QModelIndex&, const QVector<int>&) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, context, [trigger](const QModelIndex&, int, int) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, context, [trigger](const QModelIndex&, int, int) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::modelReset, context, [trigger]() mutable {
        trigger();
    });
}

template <typename RefreshFn, typename... Models>
void bindSelectionRefreshes(QObject* context, RefreshFn&& refresh, Models&... models)
{
    (bindSelectionRefresh(models, context, refresh), ...);
}

}

SessionSelection::SessionSelection(SessionModels& models, QObject* parent)
    : QObject(parent)
    , models_(models)
    , state_(models_.actors(),
             models_.properties(),
             models_.contracts(),
             models_.statements(),
             models_.transactions(),
             models_.analyses(),
             models_.annuals())
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
    bindSelectionRefreshes(this,
                           [this]() {
                               refreshSelections();
                           },
                           models_.actors(),
                           models_.properties(),
                           models_.contracts(),
                           models_.statements(),
                           models_.transactions(),
                           models_.analyses(),
                           models_.annuals());
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
