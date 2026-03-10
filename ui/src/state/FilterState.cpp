#include "ui/state/FilterState.h"

#include <QObject>

#include "ui/models/TransactionFilter.h"
#include "ui/models/TransactionList.h"

namespace ui {

namespace {

TransactionFilter* ensureFilter(QHash<QString, TransactionFilter*>& filters,
                                const QString& key,
                                TransactionList& sourceModel,
                                QObject* parent,
                                const std::function<void(TransactionFilter&)>& configure)
{
    if (key.isEmpty() || !parent) return nullptr;
    if (filters.contains(key)) return filters.value(key);

    auto* proxy = new TransactionFilter(parent);
    proxy->setSourceModel(&sourceModel);
    configure(*proxy);
    filters.insert(key, proxy);
    return proxy;
}

}

TransactionFilter* FilterState::statementTransactions(const QString& statementId, TransactionList& sourceModel, QObject* parent)
{
    return ensureFilter(statementFilters_, statementId, sourceModel, parent, [&statementId](TransactionFilter& filter) {
        filter.setStatementId(statementId);
    });
}

TransactionFilter* FilterState::propertyTransactions(const QString& propertyId, TransactionList& sourceModel, QObject* parent)
{
    return ensureFilter(propertyFilters_, propertyId, sourceModel, parent, [&propertyId](TransactionFilter& filter) {
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
