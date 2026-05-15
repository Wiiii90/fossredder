/**
 * @file ui/src/state/FilterState.cpp
 * @brief Implementation of the UI FilterState component.
 */

#include "ui/state/filters/FilterState.h"

#include <QObject>

#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/viewmodels/booking/TransactionListModel.h"

namespace ui {

namespace {

TransactionFilter* ensureFilter(QHash<QString, TransactionFilter*>& filters,
                                const QString& key,
                                TransactionList& sourceModel,
                                QObject* owner,
                                const std::function<void(TransactionFilter&)>& configure)
{
    if (key.isEmpty() || !owner) return nullptr;
    if (filters.contains(key)) return filters.value(key);

    auto* proxy = new TransactionFilter(owner);
    proxy->setSourceModel(&sourceModel);
    configure(*proxy);
    filters.insert(key, proxy);
    return proxy;
}

}

FilterState::FilterState(QObject* owner)
    : owner_(owner)
{
}

TransactionFilter* FilterState::statementTransactions(const QString& statementId, TransactionList& sourceModel)
{
    return ensureFilter(statementFilters_, statementId, sourceModel, owner_, [&statementId](TransactionFilter& filter) {
        filter.setStatementId(statementId);
    });
}

TransactionFilter* FilterState::propertyTransactions(const QString& propertyId, TransactionList& sourceModel)
{
    return ensureFilter(propertyFilters_, propertyId, sourceModel, owner_, [&propertyId](TransactionFilter& filter) {
        filter.setPropertyId(propertyId);
    });
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
