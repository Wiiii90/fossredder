#include "ui/models/ContractList.h"

#include <QVariant>

#include "ui/controllers/ControllerStrings.h"
#include "ui/payload/ProjectionConverters.h"
#include "ui/payload/PayloadKeys.h"

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
    return QString::fromStdString(c->id);
  case NameRole:
    return QString::fromStdString(c->name);
  case TypeRole:
    return QString::fromStdString(c->type);
  case DescriptionRole:
    return QString::fromStdString(c->description);
  case StartDateRole:
    return QString::fromStdString(c->startDate);
  case EndDateRole:
    return QString::fromStdString(c->endDate);
  case BasePriceRole:
    return c->basePrice;
  case ConsumptionPriceRole:
    return c->consumptionPrice;
  case MonthlyAdvanceRole:
    return c->monthlyAdvance;
  case ActorIdsRole:
    return payload::projection::toQStringList(c->actorIds);
  case PropertyIdsRole:
    return payload::projection::toQStringList(c->propertyIds);
  default:
    return {};
  }
}

QVariantMap ContractList::get(int index) const {
  QVariantMap m;
  const auto &c = itemAtRow(index);
  if (!c) return m;
  m[ui::payload::keys::common::kId] = QString::fromStdString(c->id);
  m[ui::payload::keys::common::kName] = QString::fromStdString(c->name);
  m[ui::payload::keys::common::kType] = QString::fromStdString(c->type);
  m[ui::payload::keys::common::kDescription] = QString::fromStdString(c->description);
  m[ui::payload::keys::contract::kStartDate] = QString::fromStdString(c->startDate);
  m[ui::payload::keys::contract::kEndDate] = QString::fromStdString(c->endDate);
  m[ui::payload::keys::contract::kBasePrice] = c->basePrice;
  m[ui::payload::keys::contract::kConsumptionPrice] = c->consumptionPrice;
  m[ui::payload::keys::contract::kMonthlyAdvance] = c->monthlyAdvance;
  m[ui::payload::keys::contract::kActorIds] = payload::projection::toQStringList(c->actorIds);
  m[ui::payload::keys::transaction::kPropertyIds] = payload::projection::toQStringList(c->propertyIds);
  return m;
}

QHash<int, QByteArray> ContractList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
  roles[DescriptionRole] = ui::payload::keys::common::kDescription.toUtf8();
  roles[StartDateRole] = ui::payload::keys::contract::kStartDate.toUtf8();
  roles[EndDateRole] = ui::payload::keys::contract::kEndDate.toUtf8();
  roles[BasePriceRole] = ui::payload::keys::contract::kBasePrice.toUtf8();
  roles[ConsumptionPriceRole] =
      ui::payload::keys::contract::kConsumptionPrice.toUtf8();
  roles[MonthlyAdvanceRole] =
      ui::payload::keys::contract::kMonthlyAdvance.toUtf8();
  roles[ActorIdsRole] = ui::payload::keys::contract::kActorIds.toUtf8();
  roles[PropertyIdsRole] =
      ui::payload::keys::transaction::kPropertyIds.toUtf8();
  return roles;
}

int ContractList::addContract(const QString &name, const QString &type,
                              const QString &description) {
  auto c = std::make_shared<Contract>();
  c->name = strings::toStdString(name);
  c->type = strings::toStdString(type);
  c->description = strings::toStdString(description);
  return appendItem(std::move(c));
}

} // namespace ui
