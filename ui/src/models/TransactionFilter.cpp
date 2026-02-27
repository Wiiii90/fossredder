#include "ui/models/TransactionFilter.h"

#include "ui/models/TransactionList.h"

TransactionFilter::TransactionFilter(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void TransactionFilter::setStatementId(const QString& id)
{
    if (statementId_ == id) return;
    statementId_ = id;
    invalidateFilter();
    emit statementIdChanged();
}

void TransactionFilter::setPropertyId(const QString& id)
{
    if (propertyId_ == id) return;
    propertyId_ = id;
    invalidateFilter();
    emit propertyIdChanged();
}

void TransactionFilter::setTxType(const QString& t)
{
    if (txType_ == t) return;
    txType_ = t;
    invalidateFilter();
    emit txTypeChanged();
}

bool TransactionFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!statementId_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto sid = sourceModel()->data(idx, TransactionList::StatementIdRole).toString();
        if (sid != statementId_) return false;
    }

    if (!propertyId_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto props = sourceModel()->data(idx, TransactionList::PropertyIdsRole).toList();
        bool found = false;
        for (const auto& v : props) {
            if (v.toString() == propertyId_) { found = true; break; }
        }
        if (!found) return false;
    }

    if (!txType_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto typ = sourceModel()->data(idx, TransactionList::TypeRole).toString();
        if (typ != txType_) return false;
    }

    if (statementId_.isEmpty() && propertyId_.isEmpty() && txType_.isEmpty()) return true;

    return true;
}
