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
    case LogIdRole: return r->logId;
    case TimeRole: return r->time;
    case TypeRole: return r->type;
    case FileRole: return r->file;
    case StatusRole: return r->status;
    case MessageRole: return r->message;
    case DraftAttachedRole: return r->draftAttached;
    case DraftIdRole: return r->draftId;
    case StatementIdRole: return r->statementId;
    default: return {};
    }
}

QHash<int, QByteArray> ImportRunList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LogIdRole] = ui::payload::keys::importRun::kLogId.toUtf8();
    roles[TimeRole] = ui::payload::keys::importRun::kTime.toUtf8();
    roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
    roles[FileRole] = ui::payload::keys::importRun::kFile.toUtf8();
    roles[StatusRole] = ui::payload::keys::common::kStatus.toUtf8();
    roles[MessageRole] = ui::payload::keys::importRun::kMessage.toUtf8();
    roles[DraftAttachedRole] = ui::payload::keys::importRun::kDraftAttached.toUtf8();
    roles[DraftIdRole] = ui::payload::keys::importRun::kDraftId.toUtf8();
    roles[StatementIdRole] = ui::payload::keys::importRun::kStatementId.toUtf8();
    return roles;
}

void ImportRunList::addRun(QString time, QString type, QString file, QString status, QString message, bool draftAttached,
                           QString statementId, QString logId)
{
    appendRow({std::move(logId), std::move(time), std::move(type), std::move(file), std::move(status), std::move(message), draftAttached, QString(), std::move(statementId)});
}

bool ImportRunList::upsertRun(const ImportRunRow& row)
{
    const int idx = findByLogId(row.logId);
    if (idx < 0) {
        appendRow(row);
        return true;
    }
    replaceRow(idx, row);
    emitRowChanged(idx, {LogIdRole, TimeRole, TypeRole, FileRole, StatusRole, MessageRole, DraftAttachedRole, DraftIdRole, StatementIdRole});
    return false;
}

int ImportRunList::findByLogId(const QString& logId) const
{
    if (logId.isEmpty()) return -1;
    const auto& items = rows();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (items[static_cast<size_t>(i)].logId == logId) return i;
    }
    return -1;
}

ImportRunRow ImportRunList::at(int index) const
{
    const auto* row = rowPtr(index);
    return row ? *row : ImportRunRow{};
}

std::vector<ImportRunRow> ImportRunList::snapshot() const
{
    return rows();
}

void ImportRunList::setRuns(std::vector<ImportRunRow> runs)
{
    setRows(std::move(runs));
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
