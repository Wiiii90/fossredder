#include "ui/models/TransactionFilterModel.h"

#include "ui/models/TransactionList.h"

TransactionFilterModel::TransactionFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void TransactionFilterModel::setStatementId(const QString& id)
{
    if (statementId_ == id) return;
    statementId_ = id;
    invalidateFilter();
    emit statementIdChanged();
}

void TransactionFilterModel::setPropertyId(const QString& id)
{
    if (propertyId_ == id) return;
    propertyId_ = id;
    invalidateFilter();
    emit propertyIdChanged();
}

void TransactionFilterModel::setTxType(const QString& t)
{
    if (txType_ == t) return;
    txType_ = t;
    invalidateFilter();
    emit txTypeChanged();
}

bool TransactionFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    // if statementId is provided, filter by statementId
    if (!statementId_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto sid = sourceModel()->data(idx, TransactionList::StatementIdRole).toString();
        if (sid != statementId_) return false;
    }

    // if propertyId provided, ensure transaction's propertyIds contains it
    if (!propertyId_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto props = sourceModel()->data(idx, TransactionList::PropertyIdsRole).toList();
        bool found = false;
        for (const auto& v : props) {
            if (v.toString() == propertyId_) { found = true; break; }
        }
        if (!found) return false;
    }

    // if txType set, filter by transaction "type" role
    if (!txType_.isEmpty()) {
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto typ = sourceModel()->data(idx, TransactionList::TypeRole).toString();
        if (typ != txType_) return false;
    }

    // if neither provided, default to false to avoid listing everything
    if (statementId_.isEmpty() && propertyId_.isEmpty() && txType_.isEmpty()) return false;

    return true;
}
