#include "ui/state/UiTransactionQueries.h"

#include "ui/models/TransactionFilter.h"
#include "ui/models/TransactionList.h"

QObject* UiTransactionQueries::transactionsForStatement(const QString& statementId, TransactionList* sourceModel, QObject* parent)
{
    if (statementId.isEmpty() || !sourceModel || !parent) return nullptr;
    if (byStatement_.contains(statementId)) {
        return byStatement_.value(statementId);
    }

    auto* proxy = new TransactionFilter(parent);
    proxy->setSourceModel(sourceModel);
    proxy->setStatementId(statementId);
    byStatement_.insert(statementId, proxy);
    return proxy;
}

QObject* UiTransactionQueries::transactionsForProperty(const QString& propertyId, TransactionList* sourceModel, QObject* parent)
{
    if (propertyId.isEmpty() || !sourceModel || !parent) return nullptr;
    if (byProperty_.contains(propertyId)) {
        return byProperty_.value(propertyId);
    }

    auto* proxy = new TransactionFilter(parent);
    proxy->setSourceModel(sourceModel);
    proxy->setPropertyId(propertyId);
    byProperty_.insert(propertyId, proxy);
    return proxy;
}

void UiTransactionQueries::clear()
{
    byStatement_.clear();
    byProperty_.clear();
}

void UiTransactionQueries::removePropertyCache(const QString& propertyId)
{
    if (propertyId.isEmpty()) return;
    byProperty_.remove(propertyId);
}
