#include "ui/models/ActorList.h"

#include <QVariant>

namespace ui {

ActorList::ActorList(QObject* parent) : QAbstractListModel(parent) {}

int ActorList::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(actors_.size());
}

QVariant ActorList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(actors_.size())) return {};
    const auto& a = actors_[row];
    if (!a) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(a->id);
    case NameRole: return QString::fromStdString(a->name);
    case TypeRole: return QString::fromStdString(a->type);
    case DescriptionRole: return QString::fromStdString(a->description);
    default: return {};
    }
}

bool ActorList::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(actors_.size())) return false;
    auto& a = actors_[row];
    if (!a) return false;

    bool changed = false;
    switch (role) {
    case NameRole: {
        const auto v = value.toString().toStdString();
        if (a->name != v) { a->name = v; changed = true; }
        break;
    }
    case TypeRole: {
        const auto v = value.toString().toStdString();
        if (a->type != v) { a->type = v; changed = true; }
        break;
    }
    case DescriptionRole: {
        const auto v = value.toString().toStdString();
        if (a->description != v) { a->description = v; changed = true; }
        break;
    }
    default:
        return false;
    }

    if (changed) emit dataChanged(index, index, {role});
    return changed;
}

Qt::ItemFlags ActorList::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> ActorList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[DescriptionRole] = "description";
    return roles;
}

void ActorList::setActors(std::vector<std::shared_ptr<Actor>> actors) {
    beginResetModel();
    actors_ = std::move(actors);
    endResetModel();
}

const std::vector<std::shared_ptr<Actor>>& ActorList::actors() const {
    return actors_;
}

int ActorList::addActor(const QString& name, const QString& type, const QString& description) {
    auto a = std::make_shared<Actor>();
    a->name = name.toStdString();
    a->type = type.toStdString();
    a->description = description.toStdString();

    const int row = static_cast<int>(actors_.size());
    beginInsertRows(QModelIndex(), row, row);
    actors_.push_back(std::move(a));
    endInsertRows();

    return row;
}

void ActorList::removeAt(int row) {
    if (row < 0 || row >= static_cast<int>(actors_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    actors_.erase(actors_.begin() + row);
    endRemoveRows();
}

}
