/**
 * @file ui/src/state/SessionStore.cpp
 * @brief Implements the UI session store and its derived metrics maintenance.
 */

#include "ui/state/SessionStore.h"

#include "ui/models/TransactionFilter.h"
#include "ui/state/SessionMetricsSync.h"
#include "ui/state/SessionMutationState.h"

namespace ui {

SessionStore::SessionStore(QObject* parent)
    : QObject(parent)
    , filters_(this)
    , models_(this)
{
    bindModelSignals();
}

void SessionStore::bindModelSignals()
{
    bindSessionMetricSignals(models_,
                             this,
                             [this]() { recomputeAllMetrics(); },
                             [this](int first, int last) { recomputeMetricsForRows(first, last); },
                             [this]() { metrics_.clearCache(); },
                             [this]() { notifyTransactionSumsForAllProperties(); });
}

void SessionStore::recomputeAllMetrics()
{
    recomputeAllSessionMetrics(models_, metrics_, [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
}

void SessionStore::recomputeMetricsForRows(int firstRow, int lastRow)
{
    recomputeSessionMetricsForRows(firstRow, lastRow, models_, metrics_, [this](const QString& id) {
        emit transactionSumsUpdated(id);
    });
}

void SessionStore::notifyTransactionSumsForAllProperties()
{
    notifySessionMetricsForAllProperties(models_, [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
}

void SessionStore::loadFromState(const AppState& state)
{
    filters_.clear();
    models_.loadFromState(state);
    propertyNames_.rebuild(models_.properties());
    recomputeAllMetrics();
}

TransactionFilter* SessionStore::statementTransactions(const QString& statementId)
{
    return filters_.statementTransactions(statementId, models_.transactions());
}

TransactionFilter* SessionStore::propertyTransactions(const QString& propertyId)
{
    return filters_.propertyTransactions(propertyId, models_.transactions());
}

QStringList SessionStore::propertyContractTypes(const QString& propertyId) const
{
    return metrics_.propertyContractTypes(propertyId, models_.transactions(), models_.contracts());
}

QVariantMap SessionStore::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return metrics_.propertyTransactionSums(propertyId, contractType, models_.transactions(), models_.contracts());
}

QString SessionStore::propertyName(const QString& id) const
{
    return propertyNames_.name(id);
}

void SessionStore::applyDeletionImpact(const DeletionImpact& impact)
{
    SessionMutationState::applyDeletionImpact(impact,
                                              models_,
                                              filters_,
                                              metrics_,
                                              propertyNames_);
    models_.refreshContractTypes();
}

void SessionStore::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    SessionMutationState::setTransactionPropertyIdsImmediate(txId,
                                                             propertyIds,
                                                             models_,
                                                             metrics_,
                                                             [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
}

}
