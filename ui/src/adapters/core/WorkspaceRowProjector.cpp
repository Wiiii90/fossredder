/**
 * @file ui/src/adapters/core/WorkspaceRowProjector.cpp
 * @brief Implements helpers that project workspace data into QML-friendly rows.
 */

#include "ui/adapters/core/WorkspaceRowProjector.h"

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/state/mutation/SessionMutationState.h"
#include "ui/state/session/WorkspaceSessionState.h"

#include "core/constants/json.h"

#include <QStringList>
#include <QVariantMap>
#include <algorithm>

namespace ui {

QString deleteNextSelectionIdForRows(const QVariantList& rows,
                                     const QString& removedId,
                                     int fallbackIndex,
                                     const QString& idKey);

namespace {

QVariantMap selectionStateMap(int currentIndex, const QString& selectedId, const QString& idKey)
{
    QVariantMap out;
    out.insert(QStringLiteral("index"), currentIndex);
    out.insert(QStringLiteral("id"), selectedId);
    out.insert(QStringLiteral("idKey"), idKey);
    return out;
}

QString rowIdAt(const QVariantList& rows, int index, const QString& idKey)
{
    if (rows.isEmpty() || index < 0 || index >= rows.size()) {
        return {};
    }
    return rows.at(index).toMap().value(idKey).toString();
}

int rowIndexOfId(const QVariantList& rows, const QString& id, const QString& idKey)
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

QVariantList uniqueConcat(const QVariantList& first, const QVariantList& second)
{
    QVariantList out = first;
    for (const auto& value : second) {
        if (!out.contains(value)) {
            out.push_back(value);
        }
    }
    return out;
}

QString serializeAdjustmentsJson(const core::domain::Analysis& analysis)
{
    QStringList entries;
    entries.reserve(static_cast<int>(analysis.adjustments().size()));
    for (const auto& [id, amount] : analysis.adjustments()) {
        entries.push_back(QString::fromLatin1("\"%1\":%2")
                              .arg(QString::fromStdString(id),
                                   QString::number(amount, 'g', 16)));
    }
    return QString::fromLatin1("{%1}").arg(entries.join(QStringLiteral(",")));
}

} // namespace

QVariantList pruneAndAppendMissingIds(const QVariantList& preferredIds, const QVariantList& availableIds)
{
    QVariantList out;
    for (const auto& id : preferredIds) {
        if (availableIds.contains(id) && !out.contains(id)) {
            out.push_back(id);
        }
    }
    for (const auto& id : availableIds) {
        if (!out.contains(id)) {
            out.push_back(id);
        }
    }
    return out;
}

int selectionIndexOfId(const QVariantList& rows, const QString& id, const QString& idKey)
{
    return rowIndexOfId(rows, id, idKey);
}

int selectionIndexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value)
{
    for (int i = 0; i < rows.size(); ++i) {
        if (rows.at(i).toMap().value(key) == value) {
            return i;
        }
    }
    return -1;
}

int selectionIndexOfString(const QVariantList& values, const QString& value)
{
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
    return std::clamp(index, 0, count - 1);
}

int wrappedSelectionIndex(int index, int count)
{
    if (count <= 0) {
        return -1;
    }
    const int normalized = index % count;
    return normalized < 0 ? normalized + count : normalized;
}

QString wrappedSelectionIdAt(const QVariantList& rows, int index, const QString& idKey)
{
    const int wrapped = wrappedSelectionIndex(index, rows.size());
    if (wrapped < 0) {
        return {};
    }
    return rowIdAt(rows, wrapped, idKey);
}

QString navigatedSelectionId(const QVariantList& rows,
                             const QString& currentId,
                             int delta,
                             int fallbackIndex,
                             const QString& idKey)
{
    const int currentIndex = selectionIndexOfId(rows, currentId, idKey);
    const int baseIndex = currentIndex >= 0 ? currentIndex : fallbackIndex;
    return wrappedSelectionIdAt(rows, baseIndex + delta, idKey);
}

QVariantList selectionRowIds(const QVariantList& rows, const QString& idKey)
{
    QVariantList out;
    out.reserve(rows.size());
    for (const auto& rowValue : rows) {
        const auto row = rowValue.toMap();
        const auto id = row.value(idKey);
        if (!id.isNull() && !id.toString().isEmpty()) {
            out.push_back(id);
        }
    }
    return out;
}

QVariantList orderedRowsBySelectionIds(const QVariantList& rows,
                                      const QVariantList& orderIds,
                                      const QString& idKey)
{
    QVariantList out;
    out.reserve(rows.size());
    for (const auto& orderId : orderIds) {
        const auto orderKey = orderId.toString();
        for (const auto& rowValue : rows) {
            const auto row = rowValue.toMap();
            if (row.value(idKey).toString() == orderKey && !out.contains(rowValue)) {
                out.push_back(rowValue);
                break;
            }
        }
    }
    for (const auto& rowValue : rows) {
        if (!out.contains(rowValue)) {
            out.push_back(rowValue);
        }
    }
    return out;
}

QVariantMap resolveSelectionRowState(const QVariantList& rows,
                                     int currentIndex,
                                     const QString& selectedId,
                                     const QString& idKey)
{
    QVariantMap out = selectionStateMap(currentIndex, selectedId, idKey);
    out.insert(QStringLiteral("currentId"), rowIdAt(rows, currentIndex, idKey));
    return out;
}

QVariantList orderWithInsertedSelectionId(const QVariantList& currentOrder,
                                         const QVariantList& availableIds,
                                         const QString& insertedId,
                                         int insertAfterIndex)
{
    QVariantList out = pruneAndAppendMissingIds(currentOrder, availableIds);
    if (!insertedId.isEmpty() && !out.contains(insertedId)) {
        const int insertIndex = std::clamp(insertAfterIndex + 1, 0, static_cast<int>(out.size()));
        out.insert(insertIndex, insertedId);
    }
    return out;
}

QVariantMap orderedSelectionRowsState(const QVariantList& rows,
                                      const QVariantList& preferredOrder,
                                      const QString& idKey)
{
    QVariantMap out;
    out.insert(QStringLiteral("rows"), orderedRowsBySelectionIds(rows, preferredOrder, idKey));
    out.insert(QStringLiteral("order"), pruneAndAppendMissingIds(preferredOrder, selectionRowIds(rows, idKey)));
    return out;
}

QVariantMap orderedSelectionStateForRows(const QVariantList& rows,
                                         const QVariantList& preferredOrder,
                                         int currentIndex,
                                         const QString& selectedId,
                                         const QString& idKey)
{
    QVariantMap out = orderedSelectionRowsState(rows, preferredOrder, idKey);
    out.insert(QStringLiteral("selection"), resolveSelectionRowState(rows, currentIndex, selectedId, idKey));
    return out;
}

QString deleteNextSelectionIdForRows(const QVariantList& rows,
                                     const QString& removedId,
                                     int fallbackIndex,
                                     const QString& idKey)
{
    const int removedIndex = selectionIndexOfId(rows, removedId, idKey);
    const int nextIndex = removedIndex >= 0 ? removedIndex + 1 : fallbackIndex;
    return wrappedSelectionIdAt(rows, nextIndex, idKey);
}

QVariantMap navigateSelectionDeltaState(const QVariantList& rows,
                                        int currentIndex,
                                        const QString& selectedId,
                                        int delta,
                                        int fallbackIndex,
                                        const QString& idKey)
{
    QVariantMap out = resolveSelectionRowState(rows, currentIndex, selectedId, idKey);
    out.insert(QStringLiteral("id"), navigatedSelectionId(rows, selectedId, delta, fallbackIndex, idKey));
    out.insert(QStringLiteral("index"), wrappedSelectionIndex((currentIndex >= 0 ? currentIndex : fallbackIndex) + delta, rows.size()));
    return out;
}

QVariantMap deleteReselectionStateForRows(const QVariantList& rows,
                                          const QVariantList& preferredOrder,
                                          int currentIndex,
                                          const QString& removedId,
                                          const QString& idKey)
{
    QVariantList remaining = rows;
    for (int i = 0; i < remaining.size(); ++i) {
        if (remaining.at(i).toMap().value(idKey).toString() == removedId) {
            remaining.removeAt(i);
            break;
        }
    }
    QVariantMap out = orderedSelectionStateForRows(remaining, preferredOrder, currentIndex, {}, idKey);
    out.insert(QStringLiteral("nextId"), deleteNextSelectionIdForRows(rows, removedId, currentIndex, idKey));
    return out;
}

QVariantList buildStatementTransactionIds(const SessionStore& session, const QString& statementId)
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId()) == statementId) {
            out.push_back(QString::fromStdString(transaction->id()));
        }
    }
    return out;
}

QVariantList pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds)
{
    return pruneAndAppendMissingIds(preferredIds, availableIds);
}

int indexOfId(const QVariantList& rows, const QString& id)
{
    return selectionIndexOfId(rows, id, ui::payload::keys::common::kId);
}

int indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value)
{
    return selectionIndexOfKeyValue(rows, key, value);
}

int indexOfString(const QVariantList& values, const QString& value)
{
    return selectionIndexOfString(values, value);
}

int normalizedIndex(int index, int count)
{
    return normalizedSelectionIndex(index, count);
}

int wrappedIndex(int index, int count)
{
    return wrappedSelectionIndex(index, count);
}

QString wrappedIdAt(const QVariantList& rows, int index)
{
    return wrappedSelectionIdAt(rows, index, ui::payload::keys::common::kId);
}

QString navigatedId(const QVariantList& rows,
                   const QString& currentId,
                   int delta,
                   int fallbackIndex)
{
    return navigatedSelectionId(rows, currentId, delta, fallbackIndex, ui::payload::keys::common::kId);
}

QVariantList displayRowsWithEmpty(const QVariantList& rows,
                                  const QString& emptyDisplay,
                                  const QString& displayKey)
{
    QVariantList out;

    QVariantMap empty;
    empty.insert(ui::payload::keys::common::kId, QString());
    empty.insert(ui::payload::keys::common::kDisplay, emptyDisplay);
    out.push_back(empty);

    for (const auto& rowValue : rows) {
        const QVariantMap row = rowValue.toMap();
        QVariantMap displayRow;
        displayRow.insert(ui::payload::keys::common::kId, row.value(ui::payload::keys::common::kId).toString());

        QString display = row.value(displayKey).toString();
        if (display.isEmpty()) {
            display = row.value(ui::payload::keys::common::kDisplay).toString();
        }
        if (display.isEmpty()) {
            display = row.value(ui::payload::keys::common::kName).toString();
        }
        displayRow.insert(ui::payload::keys::common::kDisplay, display);
        out.push_back(displayRow);
    }

    return out;
}

QVariantList rowIds(const QVariantList& rows, const QString& idKey)
{
    return selectionRowIds(rows, idKey);
}

QVariantList orderedRowsByIds(const QVariantList& rows,
                              const QVariantList& orderIds,
                              const QString& idKey)
{
    return orderedRowsBySelectionIds(rows, orderIds, idKey);
}

QVariantMap mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value)
{
    QVariantMap out = base;
    out.insert(key, value);
    return out;
}

QVariantMap resolveSelectionState(const QVariantList& rows,
                                  int currentIndex,
                                  const QString& selectedId,
                                  const QString& idKey)
{
    return resolveSelectionRowState(rows, currentIndex, selectedId, idKey);
}

QVariantList orderWithInsertedId(const QVariantList& currentOrder,
                                 const QVariantList& availableIds,
                                 const QString& insertedId,
                                 int insertAfterIndex)
{
    return orderWithInsertedSelectionId(currentOrder, availableIds, insertedId, insertAfterIndex);
}

QVariantMap orderedRowsState(const QVariantList& rows,
                             const QVariantList& preferredOrder,
                             const QString& idKey)
{
    return orderedSelectionRowsState(rows, preferredOrder, idKey);
}

QVariantMap orderedSelectionState(const QVariantList& rows,
                                  const QVariantList& preferredOrder,
                                  int currentIndex,
                                  const QString& selectedId,
                                  const QString& idKey)
{
    return orderedSelectionStateForRows(rows, preferredOrder, currentIndex, selectedId, idKey);
}

QVariantMap navigateSelectionState(const QVariantList& rows,
                                   int currentIndex,
                                   const QString& selectedId,
                                   int delta,
                                   int fallbackIndex,
                                   const QString& idKey)
{
    return navigateSelectionDeltaState(rows, currentIndex, selectedId, delta, fallbackIndex, idKey);
}

QVariantMap deleteReselectionState(const QVariantList& rows,
                                   const QVariantList& preferredOrder,
                                   int currentIndex,
                                   const QString& removedId,
                                   const QString& idKey)
{
    return deleteReselectionStateForRows(rows, preferredOrder, currentIndex, removedId, idKey);
}

QString deleteNextSelectionId(const QVariantList& rows,
                              const QString& removedId,
                              int fallbackIndex,
                              const QString& idKey)
{
    return deleteNextSelectionIdForRows(rows, removedId, fallbackIndex, idKey);
}

QVariantMap basicFormState(const QString& name,
                           const QVariantList& aliases,
                           const QVariantList& selectedIds)
{
    QVariantMap out;
    const QVariantList normalizedAliases = SessionMutationState::normalizeStrings(aliases);
    out.insert(ui::payload::keys::common::kName, name);
    out.insert(ui::payload::keys::actor::kAliases, normalizedAliases);
    out.insert(ui::payload::keys::state::kAliasInputText, QString());
    out.insert(ui::payload::keys::state::kAliasIndex, normalizedAliases.isEmpty() ? -1 : 0);
    out.insert(ui::payload::keys::state::kSelectedIds, SessionMutationState::normalizeStrings(selectedIds));
    return out;
}

QVariantMap contractFormState(const QString& name,
                              const QString& type,
                              const QVariantList& actorIds,
                              const QVariantList& propertyIds,
                              const QVariantList& aliases)
{
    QVariantMap out = basicFormState(name, aliases);
    const QVariantList normalizedActorIds = SessionMutationState::normalizeStrings(actorIds);
    QVariantList singleActorId;
    if (!normalizedActorIds.isEmpty()) {
        singleActorId.push_back(normalizedActorIds.first());
    }

    out.insert(ui::payload::keys::common::kType, type);
    out.insert(ui::payload::keys::state::kSelectedActorIds, singleActorId);
    out.insert(ui::payload::keys::state::kSelectedPropertyIds, SessionMutationState::normalizeStrings(propertyIds));
    return out;
}

QVariantList buildActorRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& actor : session.models().actors().actors()) {
        if (!actor) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(actor->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(actor->name()));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(actor->name()));
        QVariantList aliases;
        aliases.reserve(static_cast<int>(actor->aliases().size()));
        for (const auto& alias : actor->aliases()) {
            QVariantMap aliasMap;
            aliasMap.insert(QStringLiteral("value"), QString::fromStdString(alias.value()));
            aliasMap.insert(QStringLiteral("kind"), QString::fromStdString(alias.kind()));
            aliases.push_back(aliasMap);
        }
        row.insert(ui::payload::keys::actor::kAliases, aliases);
        out.push_back(row);
    }
    return out;
}

QVariantList buildPropertyRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& property : session.models().properties().properties()) {
        if (!property) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(property->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(property->name()));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(property->name()));
        QVariantList aliases;
        aliases.reserve(static_cast<int>(property->aliases().size()));
        for (const auto& alias : property->aliases()) {
            QVariantMap aliasMap;
            aliasMap.insert(QStringLiteral("value"), QString::fromStdString(alias.value()));
            aliasMap.insert(QStringLiteral("kind"), QString::fromStdString(alias.kind()));
            aliases.push_back(aliasMap);
        }
        row.insert(ui::payload::keys::property::kAliases, aliases);
        out.push_back(row);
    }
    return out;
}

QVariantList buildContractRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& contract : session.models().contracts().contracts()) {
        if (!contract) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(contract->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(contract->name()));
        row.insert(ui::payload::keys::common::kType, QString::fromStdString(contract->type()));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(contract->name()));
        std::vector<std::string> aliases;
        aliases.reserve(contract->aliases().size());
        for (const auto& alias : contract->aliases()) {
            aliases.push_back(alias.value());
        }
        row.insert(ui::payload::keys::contract::kAliases, payload::mapper::toVariantStringList(aliases));
        row.insert(ui::payload::keys::contract::kActorIds, payload::mapper::toQStringList(contract->actorIds()));
        row.insert(ui::payload::keys::contract::kPropertyIds, payload::mapper::toQStringList(contract->propertyIds()));
        out.push_back(row);
    }
    return out;
}

QVariantList buildAnalysisRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& analysis : session.models().analyses().analyses()) {
        if (!analysis) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(analysis->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(analysis->name()));
        row.insert(ui::payload::keys::common::kType, QString::fromStdString(analysis->type()));
        row.insert(ui::payload::keys::analysis::kConfig, QString::fromStdString(analysis->configJson()));
        row.insert(ui::payload::keys::analysis::kFilter, QString::fromStdString(analysis->filterSpec()));
        row.insert(ui::payload::keys::analysis::kAdjustments,
                   serializeAdjustmentsJson(*analysis));
        row.insert(ui::payload::keys::analysis::kExportFormat, QString::fromStdString(analysis->exportFormat()));
        row.insert(ui::payload::keys::analysis::kIncludeCalcAdjustments, analysis->includeCalculationAdjustments());
        row.insert(ui::payload::keys::analysis::kExportState, QString::fromStdString(analysis->exportStateJson()));
        row.insert(ui::payload::keys::analysis::kSnapshotTransactions, QString::fromStdString(analysis->snapshotTransactionsJson()));
        row.insert(ui::payload::keys::analysis::kCreatedAt, QString::fromStdString(analysis->createdAt()));
        row.insert(ui::payload::keys::analysis::kUpdatedAt, QString::fromStdString(analysis->updatedAt()));
        out.push_back(row);
    }
    return out;
}

QVariantList buildAnnualRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& annual : session.models().annuals().annuals()) {
        if (!annual) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(annual->id()));
        row.insert(ui::payload::keys::annual::kName, QString::fromStdString(annual->name()));
        row.insert(ui::payload::keys::annual::kYear, annual->year());
        row.insert(ui::payload::keys::annual::kAnalysisIds, payload::mapper::toVariantStringList(annual->analysisIds()));
        row.insert(ui::payload::keys::common::kDisplay,
                   annual->name().empty() ? QString::number(annual->year())
                                          : QString::fromStdString(annual->name()));
        out.push_back(row);
    }
    return out;
}

QVariantList buildStatementRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& statement : session.models().statements().statements()) {
        if (!statement) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(statement->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(statement->name()));
        out.push_back(row);
    }
    return out;
}

QVariantList buildStatementTransactionRows(const SessionStore& session, const QString& statementId)
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId()) != statementId) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(transaction->id()));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(transaction->name()));
        row.insert(ui::payload::keys::transaction::kBookingDate, QString::fromStdString(transaction->bookingDate()));
        out.push_back(row);
    }
    return out;
}

} // namespace ui

