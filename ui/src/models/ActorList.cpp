/**
 * @file ui/src/models/ActorList.cpp
 * @brief Implementation of the UI ActorList component.
 */

#include "ui/models/ActorList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"
#include "ui/util/StringConversions.h"

namespace ui {

ActorList::ActorList(QObject *parent) : Base(parent) {}

QVariant ActorList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &a = itemAtRow(index.row());
  if (!a)
    return {};

  switch (role) {
  case IdRole:
    return QString::fromStdString(a->id);
  case NameRole:
    return QString::fromStdString(a->name);
  default:
    return {};
  }
}

QHash<int, QByteArray> ActorList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  return roles;
}

int ActorList::addActor(const QString &name) {
  auto a = std::make_shared<Actor>();
  a->name = strings::toStdString(name);
  return appendItem(std::move(a));
}

} // namespace ui
