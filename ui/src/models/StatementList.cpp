#include "ui/models/StatementList.h"

#include <QVariant>

namespace ui {

void StatementList::rebuildIdIndex()
{
    idToRow_.clear();
    idToRow_.reserve(static_cast<int>(statements_.size()));
    for (int i = 0; i < static_cast<int>(statements_.size()); ++i) {
        const auto& s = statements_[static_cast<size_t>(i)];
        if (!s) continue;
        idToRow_.insert(QString::fromStdString(s->id), i);
    }
}

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
    rebuildIdIndex();
    endResetModel();
}

const std::vector<std::shared_ptr<Statement>>& StatementList::statements() const
{
    return statements_;
}

int StatementList::findRowById(const QString& id) const
{
    if (id.isEmpty()) return -1;
    const auto it = idToRow_.find(id);
    return it == idToRow_.end() ? -1 : it.value();
}

void StatementList::removeAt(int row)
{
    if (row < 0 || row >= static_cast<int>(statements_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    statements_.erase(statements_.begin() + row);
    rebuildIdIndex();
    endRemoveRows();
}

}
