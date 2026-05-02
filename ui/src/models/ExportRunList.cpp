/**
 * @file ui/src/models/ExportRunList.cpp
 * @brief Implementation of the UI ExportRunList component.
 */

#include "ui/models/ExportRunList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"

namespace ui {

ExportRunList::ExportRunList(QObject* parent)
    : Base(parent)
{
}

QVariant ExportRunList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto* r = rowPtr(index.row());
    if (!r) return {};

    switch (role) {
    case LogIdRole: return r->logId;
    case TimeRole: return r->time;
    case FileRole: return r->file;
    case StatusRole: return r->status;
    case MessageRole: return r->message;
    case PayloadRole: return r->payload;
    default: return {};
    }
}

QHash<int, QByteArray> ExportRunList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LogIdRole] = ui::payload::keys::exportRun::kLogId.toUtf8();
    roles[TimeRole] = ui::payload::keys::exportRun::kTime.toUtf8();
    roles[FileRole] = ui::payload::keys::exportRun::kFile.toUtf8();
    roles[StatusRole] = ui::payload::keys::common::kStatus.toUtf8();
    roles[MessageRole] = ui::payload::keys::exportRun::kMessage.toUtf8();
    roles[PayloadRole] = ui::payload::keys::exportRun::kPayload.toUtf8();
    return roles;
}

bool ExportRunList::upsertRun(const ExportRunRow& row)
{
    const int idx = findByLogId(row.logId);
    if (idx < 0) {
        appendRow(row);
        return true;
    }
    replaceRow(idx, row);
    emitRowChanged(idx, {LogIdRole, TimeRole, FileRole, StatusRole, MessageRole, PayloadRole});
    return false;
}

int ExportRunList::findByLogId(const QString& logId) const
{
    if (logId.isEmpty()) return -1;
    const auto& items = rows();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (items[static_cast<size_t>(i)].logId == logId) return i;
    }
    return -1;
}

ExportRunRow ExportRunList::at(int index) const
{
    const auto* row = rowPtr(index);
    return row ? *row : ExportRunRow{};
}

std::vector<ExportRunRow> ExportRunList::snapshot() const
{
    return rows();
}

void ExportRunList::setRuns(std::vector<ExportRunRow> runs)
{
    setRows(std::move(runs));
}

void ExportRunList::removeAt(int index)
{
    removeRow(index);
}

void ExportRunList::clear()
{
    clearRows();
}

}
