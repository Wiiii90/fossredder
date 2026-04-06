/**
 * @file ui/src/models/ImportRunList.cpp
 * @brief Implementation of the UI ImportRunList component.
 */

#include "ui/models/ImportRunList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"

namespace ui {

ImportRunList::ImportRunList(QObject* parent)
    : Base(parent)
{
}

QVariant ImportRunList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto* r = rowPtr(index.row());
    if (!r) return {};

    switch (role) {
    case TimeRole: return r->time;
    case TypeRole: return r->type;
    case FileRole: return r->file;
    case StatusRole: return r->status;
    case MessageRole: return r->message;
    default: return {};
    }
}

QHash<int, QByteArray> ImportRunList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimeRole] = ui::payload::keys::importRun::kTime.toUtf8();
    roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
    roles[FileRole] = ui::payload::keys::importRun::kFile.toUtf8();
    roles[StatusRole] = ui::payload::keys::common::kStatus.toUtf8();
    roles[MessageRole] = ui::payload::keys::importRun::kMessage.toUtf8();
    return roles;
}

void ImportRunList::addRun(QString time, QString type, QString file, QString status, QString message)
{
    appendRow({std::move(time), std::move(type), std::move(file), std::move(status), std::move(message)});
}

void ImportRunList::removeAt(int index)
{
    removeRow(index);
}

void ImportRunList::clear()
{
    clearRows();
}

}
