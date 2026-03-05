#include "ui/models/PropertyList.h"

#include <QVariant>

namespace ui {

PropertyList::PropertyList(QObject* parent) : QAbstractListModel(parent) {}

int PropertyList::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(props_.size());
}

QVariant PropertyList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(props_.size())) return {};
    const auto& p = props_[row];
    if (!p) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(p->id);
    case NameRole: return QString::fromStdString(p->name);
    case AddressRole: return QString::fromStdString(p->address);
    case DescriptionRole: return QString::fromStdString(p->description);
    case ConsumptionRole: return p->consumption;
    case ConsumptionUnitRole: return QString::fromStdString(p->consumptionUnit);
    default: return {};
    }
}

bool PropertyList::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(props_.size())) return false;
    auto& p = props_[row];
    if (!p) return false;

    bool changed = false;
    switch (role) {
    case NameRole: {
        const auto v = value.toString().toStdString();
        if (p->name != v) { p->name = v; changed = true; }
        break;
    }
    case AddressRole: {
        const auto v = value.toString().toStdString();
        if (p->address != v) { p->address = v; changed = true; }
        break;
    }
    case DescriptionRole: {
        const auto v = value.toString().toStdString();
        if (p->description != v) { p->description = v; changed = true; }
        break;
    }
    case ConsumptionRole: {
        const auto v = value.toDouble();
        if (p->consumption != v) { p->consumption = v; changed = true; }
        break;
    }
    case ConsumptionUnitRole: {
        const auto v = value.toString().toStdString();
        if (p->consumptionUnit != v) { p->consumptionUnit = v; changed = true; }
        break;
    }
    default:
        return false;
    }

    if (changed) emit dataChanged(index, index, {role});
    return changed;
}

Qt::ItemFlags PropertyList::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> PropertyList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[AddressRole] = "address";
    roles[DescriptionRole] = "description";
    roles[ConsumptionRole] = "consumption";
    roles[ConsumptionUnitRole] = "consumptionUnit";
    return roles;
}

void PropertyList::setProperties(std::vector<std::shared_ptr<Property>> props) {
    beginResetModel();
    props_ = std::move(props);
    endResetModel();
}

const std::vector<std::shared_ptr<Property>>& PropertyList::properties() const {
    return props_;
}

int PropertyList::addProperty(const QString& name, const QString& address, const QString& description) {
    auto p = std::make_shared<Property>();
    p->name = name.toStdString();
    p->address = address.toStdString();
    p->description = description.toStdString();

    const int row = static_cast<int>(props_.size());
    beginInsertRows(QModelIndex(), row, row);
    props_.push_back(std::move(p));
    endInsertRows();

    return row;
}

void PropertyList::removeAt(int row) {
    if (row < 0 || row >= static_cast<int>(props_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    props_.erase(props_.begin() + row);
    endRemoveRows();
}

}
