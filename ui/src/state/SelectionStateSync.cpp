/**
 * @file ui/src/state/SelectionStateSync.cpp
 * @brief Implements helpers for selection refresh tracking.
 */

#include "ui/state/SelectionStateSync.h"

#include <algorithm>
#include <QSet>

#include "ui/payload/PayloadKeys.h"
#include "ui/state/SessionSelection.h"

namespace ui {

SelectionIdsSnapshot captureSelectionIds(const SelectionState& selection)
{
    return {
        selection.selectedActorId(),
        selection.selectedPropertyId(),
        selection.selectedContractId(),
        selection.selectedStatementId(),
        selection.selectedTransactionId(),
        selection.selectedAnalysisId(),
        selection.selectedAnnualId()
    };
}

void emitSelectionChanges(SessionSelection& selection, const SelectionIdsSnapshot& before)
{
    if (selection.selectedActorId() != before.actorId) emit selection.selectedActorIdChanged();
    if (selection.selectedPropertyId() != before.propertyId) emit selection.selectedPropertyIdChanged();
    if (selection.selectedContractId() != before.contractId) emit selection.selectedContractIdChanged();
    if (selection.selectedStatementId() != before.statementId) emit selection.selectedStatementIdChanged();
    if (selection.selectedTransactionId() != before.transactionId) emit selection.selectedTransactionIdChanged();
    if (selection.selectedAnalysisId() != before.analysisId) emit selection.selectedAnalysisIdChanged();
    if (selection.selectedAnnualId() != before.annualId) emit selection.selectedAnnualIdChanged();
}

int selectionIndexOfId(const QVariantList& rows, const QString& id, const QString& idKey)
{
    if (id.isEmpty()) {
        return -1;
    }

    for (int i = 0; i < rows.size(); ++i) {
        const QVariantMap row = rows.at(i).toMap();
        if (row.value(idKey).toString() == id) {
            return i;
        }
    }
    return -1;
}

int selectionIndexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value)
{
    for (int i = 0; i < rows.size(); ++i) {
        const QVariantMap row = rows.at(i).toMap();
        if (row.value(key) == value) {
            return i;
        }
    }
    return -1;
}

int selectionIndexOfString(const QVariantList& values, const QString& value)
{
    if (value.isEmpty()) {
        return -1;
    }

    for (int i = 0; i < values.size(); ++i) {
        if (values.at(i).toString() == value) {
            return i;
        }
    }
    return -1;
}

int normalizedSelectionIndex(int index, int count)
{
    if (count <= 0) {
        return -1;
    }

    return std::max(0, std::min(index, count - 1));
}

int wrappedSelectionIndex(int index, int count)
{
    if (count <= 0) {
        return -1;
    }

    int idx = index;
    while (idx < 0) {
        idx += count;
    }
    return idx % count;
}

QString wrappedSelectionIdAt(const QVariantList& rows, int index, const QString& idKey)
{
    if (rows.isEmpty()) {
        return QString();
    }

    int idx = index;
    while (idx < 0) {
        idx += rows.size();
    }
    idx %= rows.size();

    return rows.at(idx).toMap().value(idKey).toString();
}

QString navigatedSelectionId(const QVariantList& rows,
                             const QString& currentId,
                             int delta,
                             int fallbackIndex,
                             const QString& idKey)
{
    if (rows.isEmpty()) {
        return QString();
    }

    const int current = selectionIndexOfId(rows, currentId, idKey);
    if (current < 0) {
        return wrappedSelectionIdAt(rows, fallbackIndex, idKey);
    }

    const int next = wrappedSelectionIndex(current + delta, rows.size());
    return wrappedSelectionIdAt(rows, next, idKey);
}

QVariantList pruneAndAppendMissingIds(const QVariantList& preferredIds, const QVariantList& availableIds)
{
    QSet<QString> available;
    available.reserve(availableIds.size());
    for (const auto& id : availableIds) {
        available.insert(id.toString());
    }

    QVariantList out;
    QSet<QString> used;
    for (const auto& id : preferredIds) {
        const QString value = id.toString();
        if (value.isEmpty() || !available.contains(value) || used.contains(value)) {
            continue;
        }
        used.insert(value);
        out.push_back(value);
    }

    for (const auto& id : availableIds) {
        const QString value = id.toString();
        if (value.isEmpty() || used.contains(value)) {
            continue;
        }
        used.insert(value);
        out.push_back(value);
    }

    return out;
}

QVariantList selectionRowIds(const QVariantList& rows, const QString& idKey)
{
    QVariantList out;
    out.reserve(rows.size());
    for (const auto& rowValue : rows) {
        const QVariantMap row = rowValue.toMap();
        const QString id = row.value(idKey).toString();
        if (!id.isEmpty()) {
            out.push_back(id);
        }
    }
    return out;
}

QVariantList orderedRowsBySelectionIds(const QVariantList& rows,
                                       const QVariantList& orderIds,
                                       const QString& idKey)
{
    QVariantMap byId;
    for (const auto& rowValue : rows) {
        const QVariantMap row = rowValue.toMap();
        const QString id = row.value(idKey).toString();
        if (!id.isEmpty()) {
            byId.insert(id, rowValue);
        }
    }

    QVariantList out;
    out.reserve(orderIds.size());
    for (const auto& idValue : orderIds) {
        const QString id = idValue.toString();
        if (!id.isEmpty() && byId.contains(id)) {
            out.push_back(byId.value(id));
        }
    }
    return out;
}

QVariantMap resolveSelectionRowState(const QVariantList& rows,
                                     int currentIndex,
                                     const QString& selectedId,
                                     const QString& idKey)
{
    QVariantMap out;
    if (rows.isEmpty()) {
        out.insert(ui::payload::keys::state::kIndex, -1);
        out.insert(ui::payload::keys::common::kId, QString());
        return out;
    }

    int index = currentIndex;
    if (!selectedId.isEmpty()) {
        index = selectionIndexOfId(rows, selectedId, idKey);
    }
    index = normalizedSelectionIndex(index, rows.size());

    const QVariantMap row = rows.at(index).toMap();
    out.insert(ui::payload::keys::state::kIndex, index);
    out.insert(ui::payload::keys::common::kId, row.value(idKey).toString());
    return out;
}

QVariantList orderWithInsertedSelectionId(const QVariantList& currentOrder,
                                          const QVariantList& availableIds,
                                          const QString& insertedId,
                                          int insertAfterIndex)
{
    const QVariantList pruned = pruneAndAppendMissingIds(currentOrder, availableIds);
    QVariantList withoutInserted;
    withoutInserted.reserve(pruned.size());
    for (const auto& id : pruned) {
        if (id.toString() != insertedId) {
            withoutInserted.push_back(id.toString());
        }
    }
    const int insertIndex = normalizedSelectionIndex(insertAfterIndex + 1, withoutInserted.size() + 1);
    withoutInserted.insert(std::max(0, insertIndex), insertedId);
    return pruneAndAppendMissingIds(withoutInserted, availableIds);
}

QVariantMap orderedSelectionRowsState(const QVariantList& rows,
                                      const QVariantList& preferredOrder,
                                      const QString& idKey)
{
    QVariantMap out;
    if (rows.isEmpty()) {
        out.insert(ui::payload::keys::state::kOrderIds, QVariantList());
        out.insert(ui::payload::keys::state::kRows, QVariantList());
        return out;
    }

    const QVariantList availableIds = selectionRowIds(rows, idKey);
    const QVariantList orderIds = pruneAndAppendMissingIds(preferredOrder, availableIds);
    out.insert(ui::payload::keys::state::kOrderIds, orderIds);
    out.insert(ui::payload::keys::state::kRows, orderedRowsBySelectionIds(rows, orderIds, idKey));
    return out;
}

QVariantMap orderedSelectionStateForRows(const QVariantList& rows,
                                         const QVariantList& preferredOrder,
                                         int currentIndex,
                                         const QString& selectedId,
                                         const QString& idKey)
{
    QVariantMap out;
    const QVariantMap ordered = orderedSelectionRowsState(rows, preferredOrder, idKey);
    const QVariantList orderIds = ordered.value(ui::payload::keys::state::kOrderIds).toList();
    const QVariantList orderedRows = ordered.value(ui::payload::keys::state::kRows).toList();

    out.insert(ui::payload::keys::state::kOrderIds, orderIds);
    out.insert(ui::payload::keys::state::kRows, orderedRows);

    const QVariantMap selection = resolveSelectionRowState(orderedRows, currentIndex, selectedId, idKey);
    out.insert(ui::payload::keys::state::kIndex, selection.value(ui::payload::keys::state::kIndex));
    out.insert(ui::payload::keys::common::kId, selection.value(ui::payload::keys::common::kId));
    return out;
}

QVariantMap navigateSelectionDeltaState(const QVariantList& rows,
                                        int currentIndex,
                                        const QString& selectedId,
                                        int delta,
                                        int fallbackIndex,
                                        const QString& idKey)
{
    QVariantMap out;
    if (rows.isEmpty()) {
        out.insert(ui::payload::keys::state::kIndex, -1);
        out.insert(ui::payload::keys::common::kId, QString());
        return out;
    }

    const QVariantMap base = resolveSelectionRowState(rows, currentIndex, selectedId, idKey);
    const int index = base.value(ui::payload::keys::state::kIndex).toInt();
    const int next = wrappedSelectionIndex(index + delta, rows.size());
    const QString id = wrappedSelectionIdAt(rows, next >= 0 ? next : fallbackIndex, idKey);

    out.insert(ui::payload::keys::state::kIndex, next >= 0 ? next : normalizedSelectionIndex(fallbackIndex, rows.size()));
    out.insert(ui::payload::keys::common::kId, id);
    return out;
}

QVariantMap deleteReselectionStateForRows(const QVariantList& rows,
                                          const QVariantList& preferredOrder,
                                          int currentIndex,
                                          const QString& removedId,
                                          const QString& idKey)
{
    QVariantMap out;
    const QVariantMap ordered = orderedSelectionRowsState(rows, preferredOrder, idKey);
    const QVariantList orderIds = ordered.value(ui::payload::keys::state::kOrderIds).toList();
    const QVariantList orderedRows = ordered.value(ui::payload::keys::state::kRows).toList();

    if (orderedRows.isEmpty()) {
        out.insert(ui::payload::keys::state::kOrderIds, orderIds);
        out.insert(ui::payload::keys::state::kIndex, -1);
        out.insert(ui::payload::keys::common::kId, QString());
        return out;
    }

    int index = normalizedSelectionIndex(currentIndex, orderedRows.size());
    if (index < 0) {
        index = 0;
    }

    QVariantMap row = orderedRows.at(index).toMap();
    QString currentId = row.value(idKey).toString();
    if (currentId.isEmpty() || currentId == removedId) {
        const int fallback = normalizedSelectionIndex(index, orderedRows.size());
        row = orderedRows.at(fallback).toMap();
        currentId = row.value(idKey).toString();
        index = fallback;
    }

    out.insert(ui::payload::keys::state::kOrderIds, orderIds);
    out.insert(ui::payload::keys::state::kIndex, index);
    out.insert(ui::payload::keys::common::kId, currentId);
    return out;
}

QString deleteNextSelectionIdForRows(const QVariantList& rows,
                                     const QString& removedId,
                                     int fallbackIndex,
                                     const QString& idKey)
{
    if (rows.isEmpty()) {
        return QString();
    }

    const int removedIndex = selectionIndexOfId(rows, removedId, idKey);
    int index = removedIndex >= 0 ? removedIndex : normalizedSelectionIndex(fallbackIndex, rows.size());
    if (index < 0) {
        index = 0;
    }

    return rows.at(index).toMap().value(idKey).toString();
}

} // namespace ui
