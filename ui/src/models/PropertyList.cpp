/**
 * @file ui/src/models/PropertyList.cpp
 * @brief Implementation of the UI PropertyList component.
 */

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
    return QString::fromStdString(p->id());
  case NameRole:
    return QString::fromStdString(p->name());
  default:
    return {};
  }
}

QHash<int, QByteArray> PropertyList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  return roles;
}

int PropertyList::addProperty(const QString &name) {
  auto p = std::make_shared<Property>();
  p->rename(strings::toStdString(name));
  return appendItem(std::move(p));
}

} // namespace ui
