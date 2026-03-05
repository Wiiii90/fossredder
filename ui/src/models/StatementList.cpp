#include "ui/models/StatementList.h"

#include <QVariant>

namespace ui {

StatementList::StatementList(QObject* parent) : QAbstractListModel(parent) {}

int StatementList::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(statements_.size());
}

QVariant StatementList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(statements_.size())) return {};
    const auto& s = statements_[row];
    if (!s) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(s->id);
    case NameRole: return QString::fromStdString(s->name);
    default: return {};
    }
}

QHash<int, QByteArray> StatementList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    return roles;
}

void StatementList::setStatements(std::vector<std::shared_ptr<Statement>> statements)
{
    beginResetModel();
    statements_ = std::move(statements);
    endResetModel();
}

const std::vector<std::shared_ptr<Statement>>& StatementList::statements() const
{
    return statements_;
}

void StatementList::removeAt(int row)
{
    if (row < 0 || row >= static_cast<int>(statements_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    statements_.erase(statements_.begin() + row);
    endRemoveRows();
}

}
