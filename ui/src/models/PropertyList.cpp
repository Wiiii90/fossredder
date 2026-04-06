#include "ui/models/PropertyList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"
#include "ui/util/StringConversions.h"

namespace ui {

PropertyList::PropertyList(QObject *parent) : Base(parent) {}

QVariant PropertyList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &p = itemAtRow(index.row());
  if (!p)
    return {};

  switch (role) {
  case IdRole:
    return QString::fromStdString(p->id);
  case NameRole:
    return QString::fromStdString(p->name);
  case AddressRole:
    return QString::fromStdString(p->address);
  case DescriptionRole:
    return QString::fromStdString(p->description);
  case ConsumptionRole:
    return p->consumption;
  case ConsumptionUnitRole:
    return QString::fromStdString(p->consumptionUnit);
  default:
    return {};
  }
}

QHash<int, QByteArray> PropertyList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  roles[AddressRole] = ui::payload::keys::property::kAddress.toUtf8();
  roles[DescriptionRole] = ui::payload::keys::common::kDescription.toUtf8();
  roles[ConsumptionRole] = ui::payload::keys::property::kConsumption.toUtf8();
  roles[ConsumptionUnitRole] =
      ui::payload::keys::property::kConsumptionUnit.toUtf8();
  return roles;
}

int PropertyList::addProperty(const QString &name, const QString &address,
                              const QString &description) {
  auto p = std::make_shared<Property>();
  p->name = strings::toStdString(name);
  p->address = strings::toStdString(address);
  p->description = strings::toStdString(description);
  return appendItem(std::move(p));
}

} // namespace ui
