/**
 * @file ui/src/models/StatementList.cpp
 * @brief Implementation of the UI StatementList component.
 */

#include "ui/models/StatementList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"

namespace ui {

StatementList::StatementList(QObject* parent) : Base(parent) {}

QVariant StatementList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto& s = itemAtRow(index.row());
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
    roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
    roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
    return roles;
}

}
