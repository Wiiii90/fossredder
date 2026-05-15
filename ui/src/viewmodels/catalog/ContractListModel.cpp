/**
 * @file ui/src/models/ContractList.cpp
 * @brief Implementation of the UI ContractList component.
 */

#include "ui/viewmodels/catalog/ContractListModel.h"

#include <QVariant>

#include "ui/shared/payload/PayloadMapper.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {

ContractList::ContractList(QObject *parent) : Base(parent) {}

QVariant ContractList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &c = itemAtRow(index.row());
  if (!c)
    return {};

  switch (role) {
  case IdRole:
    return QString::fromStdString(c->id());
  case NameRole:
    return QString::fromStdString(c->name());
  case TypeRole:
    return QString::fromStdString(c->type());
  case ActorIdsRole:
    return payload::mapper::toQStringList(c->actorIds());
  case PropertyIdsRole:
    return payload::mapper::toQStringList(c->propertyIds());
  default:
    return {};
  }
}

QVariantMap ContractList::get(int index) const {
  QVariantMap m;
  const auto &c = itemAtRow(index);
  if (!c) return m;
  m[ui::payload::keys::common::kId] = QString::fromStdString(c->id());
  m[ui::payload::keys::common::kName] = QString::fromStdString(c->name());
  m[ui::payload::keys::common::kType] = QString::fromStdString(c->type());
  m[ui::payload::keys::contract::kActorIds] = payload::mapper::toQStringList(c->actorIds());
  m[ui::payload::keys::transaction::kPropertyIds] = payload::mapper::toQStringList(c->propertyIds());
  return m;
}

QHash<int, QByteArray> ContractList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
  roles[ActorIdsRole] = ui::payload::keys::contract::kActorIds.toUtf8();
  roles[PropertyIdsRole] =
      ui::payload::keys::transaction::kPropertyIds.toUtf8();
  return roles;
}

int ContractList::addContract(const QString &name, const QString &type) {
  auto c = std::make_shared<Contract>();
  c->rename(strings::toStdString(name));
  c->setType(strings::toStdString(type));
  return appendItem(std::move(c));
}

} // namespace ui



