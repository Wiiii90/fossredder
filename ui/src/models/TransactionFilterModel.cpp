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

bool TransactionFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (statementId_.isEmpty()) return false;

    const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto sid = sourceModel()->data(idx, TransactionList::StatementIdRole).toString();
    return sid == statementId_;
}
