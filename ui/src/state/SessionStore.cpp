/**
 * @file ui/src/state/SessionStore.cpp
 * @brief Implements the UI session store and its derived metrics maintenance.
 */

#include "ui/state/SessionStore.h"

#include "ui/models/TransactionFilter.h"
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
}

void SessionStore::loadFromState(const AppState& state)
{
    filters_.clear();
    models_.loadFromState(state);
}

TransactionFilter* SessionStore::statementTransactions(const QString& statementId)
{
    return filters_.statementTransactions(statementId, models_.transactions());
}

TransactionFilter* SessionStore::propertyTransactions(const QString& propertyId)
{
    return filters_.propertyTransactions(propertyId, models_.transactions());
}

void SessionStore::applyDeletionImpact(const DeletionImpact& impact)
{
    SessionMutationState::applyDeletionImpact(impact,
                                              models_,
                                              filters_);
    models_.refreshContractTypes();
}

void SessionStore::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    SessionMutationState::setTransactionPropertyIdsImmediate(txId, propertyIds, models_);
}

}
