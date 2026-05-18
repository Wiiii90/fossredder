/**
 * @file ui/src/state/SessionStore.cpp
 * @brief Implements the UI session store and its derived metrics maintenance.
 */

#include "ui/state/session/WorkspaceSessionState.h"

#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/state/mutation/SessionMutationState.h"

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

void SessionStore::bumpDataRevision()
{
    ++dataRevision_;
    emit dataRevisionChanged();
}

void SessionStore::loadFromState(const core::domain::catalog::WorkspaceCatalog& state)
{
    filters_.clear();
    models_.loadFromState(state);
    bumpDataRevision();
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
    bumpDataRevision();
}

void SessionStore::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    SessionMutationState::setTransactionPropertyIdsImmediate(txId, propertyIds, models_);
    bumpDataRevision();
}

}

