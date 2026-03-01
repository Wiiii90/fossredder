#include "ui/state/FilterState.h"

#include <QObject>

#include "ui/models/TransactionFilter.h"
#include "ui/models/TransactionList.h"

namespace ui {

QObject* FilterState::statementTransactions(const QString& statementId, TransactionList& sourceModel, QObject* parent)
{
    if (statementId.isEmpty() || !parent) return nullptr;
    if (statementFilters_.contains(statementId)) return statementFilters_.value(statementId);
    auto* proxy = new TransactionFilter(parent);
    proxy->setSourceModel(&sourceModel);
    proxy->setStatementId(statementId);
    statementFilters_.insert(statementId, proxy);
    return proxy;
}

QObject* FilterState::propertyTransactions(const QString& propertyId, TransactionList& sourceModel, QObject* parent)
{
    if (propertyId.isEmpty() || !parent) return nullptr;
    if (propertyFilters_.contains(propertyId)) return propertyFilters_.value(propertyId);
    auto* proxy = new TransactionFilter(parent);
    proxy->setSourceModel(&sourceModel);
    proxy->setPropertyId(propertyId);
    propertyFilters_.insert(propertyId, proxy);
    return proxy;
}

void FilterState::clear()
{
    for (auto* filter : statementFilters_) if (filter) filter->deleteLater();
    for (auto* filter : propertyFilters_) if (filter) filter->deleteLater();
    statementFilters_.clear();
    propertyFilters_.clear();
}

void FilterState::removeStatement(const QString& statementId)
{
    if (statementId.isEmpty()) return;
    auto* filter = statementFilters_.value(statementId, nullptr);
    if (filter) filter->deleteLater();
    statementFilters_.remove(statementId);
}

void FilterState::removeProperty(const QString& propertyId)
{
    if (propertyId.isEmpty()) return;
    auto* filter = propertyFilters_.value(propertyId, nullptr);
    if (filter) filter->deleteLater();
    propertyFilters_.remove(propertyId);
}

}
