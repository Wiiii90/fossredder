#include "ui/models/ContractList.h"

#include <QVariant>

namespace ui {

ContractList::ContractList(QObject* parent) : QAbstractListModel(parent) {}

int ContractList::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(contracts_.size());
}

QVariant ContractList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(contracts_.size())) return {};
    const auto& c = contracts_[row];
    if (!c) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(c->id);
    case NameRole: return QString::fromStdString(c->name);
    case TypeRole: return QString::fromStdString(c->type);
    case DescriptionRole: return QString::fromStdString(c->description);
    case StartDateRole: return QString::fromStdString(c->startDate);
    case EndDateRole: return QString::fromStdString(c->endDate);
    case BasePriceRole: return c->basePrice;
    case ConsumptionPriceRole: return c->consumptionPrice;
    case MonthlyAdvanceRole: return c->monthlyAdvance;
    case ActorIdsRole: return toQStringList(c->actorIds);
    case PropertyIdsRole: return toQStringList(c->propertyIds);
    default: return {};
    }
}

bool ContractList::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(contracts_.size())) return false;
    auto& c = contracts_[row];
    if (!c) return false;

    bool changed = false;
    switch (role) {
    case NameRole: {
        const auto v = value.toString().toStdString();
        if (c->name != v) { c->name = v; changed = true; }
        break;
    }
    case TypeRole: {
        const auto v = value.toString().toStdString();
        if (c->type != v) { c->type = v; changed = true; }
        break;
    }
    case DescriptionRole: {
        const auto v = value.toString().toStdString();
        if (c->description != v) { c->description = v; changed = true; }
        break;
    }
    case StartDateRole: {
        const auto v = value.toString().toStdString();
        if (c->startDate != v) { c->startDate = v; changed = true; }
        break;
    }
    case EndDateRole: {
        const auto v = value.toString().toStdString();
        if (c->endDate != v) { c->endDate = v; changed = true; }
        break;
    }
    case BasePriceRole: {
        const auto v = value.toDouble();
        if (c->basePrice != v) { c->basePrice = v; changed = true; }
        break;
    }
    case ConsumptionPriceRole: {
        const auto v = value.toDouble();
        if (c->consumptionPrice != v) { c->consumptionPrice = v; changed = true; }
        break;
    }
    case MonthlyAdvanceRole: {
        const auto v = value.toDouble();
        if (c->monthlyAdvance != v) { c->monthlyAdvance = v; changed = true; }
        break;
    }
    case ActorIdsRole: {
        auto vv = toStdVectorStrings(value);
        if (c->actorIds != vv) { c->actorIds = std::move(vv); changed = true; }
        break;
    }
    case PropertyIdsRole: {
        auto vv = toStdVectorStrings(value);
        if (c->propertyIds != vv) { c->propertyIds = std::move(vv); changed = true; }
        break;
    }
    default:
        return false;
    }

    if (changed) emit dataChanged(index, index, {role});
    return changed;
}

Qt::ItemFlags ContractList::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> ContractList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[TypeRole] = "type";
    roles[DescriptionRole] = "description";
    roles[StartDateRole] = "startDate";
    roles[EndDateRole] = "endDate";
    roles[BasePriceRole] = "basePrice";
    roles[ConsumptionPriceRole] = "consumptionPrice";
    roles[MonthlyAdvanceRole] = "monthlyAdvance";
    roles[ActorIdsRole] = "actorIds";
    roles[PropertyIdsRole] = "propertyIds";
    return roles;
}

void ContractList::setContracts(std::vector<std::shared_ptr<Contract>> contracts) {
    beginResetModel();
    contracts_ = std::move(contracts);
    endResetModel();
}

const std::vector<std::shared_ptr<Contract>>& ContractList::contracts() const {
    return contracts_;
}

int ContractList::addContract(const QString& name, const QString& type, const QString& description) {
    auto c = std::make_shared<Contract>();
    c->name = name.toStdString();
    c->type = type.toStdString();
    c->description = description.toStdString();

    const int row = static_cast<int>(contracts_.size());
    beginInsertRows(QModelIndex(), row, row);
    contracts_.push_back(std::move(c));
    endInsertRows();

    return row;
}

void ContractList::removeAt(int row) {
    if (row < 0 || row >= static_cast<int>(contracts_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    contracts_.erase(contracts_.begin() + row);
    endRemoveRows();
}

QStringList ContractList::toQStringList(const std::vector<std::string>& v) {
    QStringList out;
    out.reserve(static_cast<int>(v.size()));
    for (const auto& s : v) out.push_back(QString::fromStdString(s));
    return out;
}

std::vector<std::string> ContractList::toStdVectorStrings(const QVariant& v) {
    std::vector<std::string> out;
    if (v.canConvert<QStringList>()) {
        const auto list = v.toStringList();
        out.reserve(list.size());
        for (const auto& s : list) out.push_back(s.toStdString());
        return out;
    }
    if (v.userType() == QMetaType::QVariantList) {
        const auto list = v.toList();
        out.reserve(list.size());
        for (const auto& item : list) out.push_back(item.toString().toStdString());
        return out;
    }
    return out;
}

}
