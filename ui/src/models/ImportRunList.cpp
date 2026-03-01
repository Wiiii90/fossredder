#include "ui/models/ImportRunList.h"

#include <QVariant>

namespace ui {

ImportRunList::ImportRunList(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ImportRunList::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(runs_.size());
}

QVariant ImportRunList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(runs_.size())) return {};
    const auto& r = runs_[row];

    switch (role) {
    case TimeRole: return r.time;
    case TypeRole: return r.type;
    case FileRole: return r.file;
    case StatusRole: return r.status;
    case MessageRole: return r.message;
    default: return {};
    }
}

QHash<int, QByteArray> ImportRunList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimeRole] = "time";
    roles[TypeRole] = "type";
    roles[FileRole] = "file";
    roles[StatusRole] = "status";
    roles[MessageRole] = "message";
    return roles;
}

void ImportRunList::addRun(QString time, QString type, QString file, QString status, QString message)
{
    beginInsertRows(QModelIndex(), static_cast<int>(runs_.size()), static_cast<int>(runs_.size()));
    runs_.push_back({std::move(time), std::move(type), std::move(file), std::move(status), std::move(message)});
    endInsertRows();
}

void ImportRunList::removeAt(int index)
{
    if (index < 0 || index >= static_cast<int>(runs_.size())) return;
    beginRemoveRows(QModelIndex(), index, index);
    runs_.erase(runs_.begin() + index);
    endRemoveRows();
}

void ImportRunList::clear()
{
    beginResetModel();
    runs_.clear();
    endResetModel();
}

}
