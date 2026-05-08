/**
 * @file ui/src/state/StateFacadeProjection.cpp
 * @brief Implements helpers that project session store data into QML-friendly rows.
 */

#include "ui/state/StateFacadeProjection.h"

#include "ui/payload/PayloadKeys.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/state/SelectionStateSync.h"
#include "ui/state/SessionMutationState.h"
#include "ui/state/SessionStore.h"

#include "core/application/AnalysisRequestComposer.h"

#include <QStringList>
#include <QVariantMap>

namespace ui {

QVariantList buildStatementTransactionIds(const SessionStore& session, const QString& statementId)
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) {
            out.push_back(QString::fromStdString(transaction->id));
        }
    }
    return out;
}

QVariantList normalizeStrings(const QVariantList& values)
{
    return SessionMutationState::normalizeStrings(values);
}

QVariantList addUniqueTrimmed(const QVariantList& values, const QString& value)
{
    return SessionMutationState::addUniqueTrimmed(values, value);
}

QVariantList removeAt(const QVariantList& values, int index)
{
    return SessionMutationState::removeAt(values, index);
}

QVariantList removeString(const QVariantList& values, const QString& value)
{
    return SessionMutationState::removeString(values, value);
}

QVariantList insertAt(const QVariantList& values, int index, const QVariant& value)
{
    return SessionMutationState::insertAt(values, index, value);
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

QVariantMap emptyTransactionDraft()
{
    QVariantMap tx;
    tx.insert(ui::payload::keys::common::kId, QString());
    tx.insert(ui::payload::keys::common::kName, QString());
    tx.insert(ui::payload::keys::transaction::kBookingDate, QString());
    tx.insert(ui::payload::keys::transaction::kValuta, QString());
    tx.insert(ui::payload::keys::common::kAmount, 0.0);
    tx.insert(ui::payload::keys::common::kDescription, QString());
    tx.insert(ui::payload::keys::common::kStatus, 0);
    tx.insert(ui::payload::keys::transaction::kActorId, QString());
    tx.insert(ui::payload::keys::transaction::kPropertyIds, QVariantList());
    tx.insert(ui::payload::keys::transaction::kAllocatable, false);
    tx.insert(ui::payload::keys::transaction::kContractId, QString());
    tx.insert(ui::payload::keys::statement::kStatementId, QString());
    return tx;
}

QVariantMap normalizeTransactionDraft(const QVariantMap& tx)
{
    QVariantMap out = emptyTransactionDraft();
    for (auto it = tx.constBegin(); it != tx.constEnd(); ++it) {
        out.insert(it.key(), it.value());
    }

    out.insert(ui::payload::keys::common::kAmount, out.value(ui::payload::keys::common::kAmount).toDouble());
    out.insert(ui::payload::keys::common::kStatus, out.value(ui::payload::keys::common::kStatus).toInt());
    out.insert(ui::payload::keys::transaction::kPropertyIds, normalizeStrings(out.value(ui::payload::keys::transaction::kPropertyIds).toList()));
    out.insert(ui::payload::keys::transaction::kActorId, out.value(ui::payload::keys::transaction::kActorId).toString());
    out.insert(ui::payload::keys::transaction::kContractId, out.value(ui::payload::keys::transaction::kContractId).toString());
    out.insert(ui::payload::keys::common::kName, out.value(ui::payload::keys::common::kName).toString());
    out.insert(ui::payload::keys::transaction::kBookingDate, out.value(ui::payload::keys::transaction::kBookingDate).toString());
    out.insert(ui::payload::keys::transaction::kValuta, out.value(ui::payload::keys::transaction::kValuta).toString());
    out.insert(ui::payload::keys::common::kDescription, out.value(ui::payload::keys::common::kDescription).toString());
    out.insert(ui::payload::keys::statement::kStatementId, out.value(ui::payload::keys::statement::kStatementId).toString());
    out.insert(ui::payload::keys::transaction::kAllocatable, out.value(ui::payload::keys::transaction::kAllocatable).toBool());
    return out;
}

QVariantList normalizeTransactionDrafts(const QVariantList& values)
{
    QVariantList out;
    out.reserve(values.size());
    for (const auto& value : values) {
        out.push_back(normalizeTransactionDraft(value.toMap()));
    }
    return out;
}

bool transactionDraftHasContent(const QVariantMap& tx)
{
    const QVariantMap normalized = normalizeTransactionDraft(tx);
    return !normalized.value(ui::payload::keys::common::kName).toString().isEmpty()
        || !normalized.value(ui::payload::keys::transaction::kBookingDate).toString().isEmpty()
        || !normalized.value(ui::payload::keys::transaction::kValuta).toString().isEmpty()
        || normalized.value(ui::payload::keys::common::kAmount).toDouble() != 0.0
        || !normalized.value(ui::payload::keys::transaction::kActorId).toString().isEmpty()
        || !normalized.value(ui::payload::keys::transaction::kPropertyIds).toList().isEmpty();
}

QVariantMap createDraftListState(const QVariantList& drafts,
                                 int currentIndex,
                                 const QVariantMap& emptyDraft)
{
    QVariantList normalizedDrafts = normalizeTransactionDrafts(drafts);
    if (normalizedDrafts.isEmpty()) {
        normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
    }

    const int index = normalizedIndex(currentIndex, normalizedDrafts.size());
    QVariantMap out;
    out.insert(ui::payload::keys::state::kDrafts, normalizedDrafts);
    out.insert(ui::payload::keys::state::kIndex, index >= 0 ? index : 0);
    return out;
}

QVariantMap insertDraftAfterCurrent(const QVariantList& drafts,
                                    int currentIndex,
                                    const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(ui::payload::keys::state::kDrafts).toList();
    const int index = base.value(ui::payload::keys::state::kIndex).toInt();
    const int insertIndex = normalizedIndex(index, normalizedDrafts.size()) + 1;
    normalizedDrafts.insert(insertIndex, normalizeTransactionDraft(emptyDraft));

    QVariantMap out;
    out.insert(ui::payload::keys::state::kDrafts, normalizedDrafts);
    out.insert(ui::payload::keys::state::kIndex, insertIndex);
    return out;
}

QVariantMap removeDraftAt(const QVariantList& drafts,
                          int currentIndex,
                          const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(ui::payload::keys::state::kDrafts).toList();
    int index = base.value(ui::payload::keys::state::kIndex).toInt();
    if (index < 0 || index >= normalizedDrafts.size()) {
        index = 0;
    }

    normalizedDrafts.removeAt(index);
    if (normalizedDrafts.isEmpty()) {
        normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
        index = 0;
    } else {
        index = normalizedIndex(index, normalizedDrafts.size());
    }

    QVariantMap out;
    out.insert(ui::payload::keys::state::kDrafts, normalizedDrafts);
    out.insert(ui::payload::keys::state::kIndex, index);
    return out;
}

QVariantMap setCurrentDraft(const QVariantList& drafts,
                            int currentIndex,
                            const QVariantMap& draft,
                            const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(ui::payload::keys::state::kDrafts).toList();
    const int index = base.value(ui::payload::keys::state::kIndex).toInt();
    normalizedDrafts[index] = normalizeTransactionDraft(draft);

    QVariantMap out;
    out.insert(ui::payload::keys::state::kDrafts, normalizedDrafts);
    out.insert(ui::payload::keys::state::kIndex, index);
    return out;
}

QVariantMap currentDraftState(const QVariantList& drafts,
                              int currentIndex,
                              const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    const QVariantList normalizedDrafts = base.value(ui::payload::keys::state::kDrafts).toList();
    const int index = base.value(ui::payload::keys::state::kIndex).toInt();

    QVariantMap out;
    out.insert(ui::payload::keys::state::kDrafts, normalizedDrafts);
    out.insert(ui::payload::keys::state::kIndex, index);
    out.insert(ui::payload::keys::state::kDraft, normalizeTransactionDraft(normalizedDrafts.at(index).toMap()));
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
    const QVariantList normalizedAliases = normalizeStrings(aliases);
    out.insert(ui::payload::keys::common::kName, name);
    out.insert(ui::payload::keys::actor::kAliases, normalizedAliases);
    out.insert(ui::payload::keys::state::kAliasInputText, QString());
    out.insert(ui::payload::keys::state::kAliasIndex, normalizedAliases.isEmpty() ? -1 : 0);
    out.insert(ui::payload::keys::state::kSelectedIds, normalizeStrings(selectedIds));
    return out;
}

QVariantMap contractFormState(const QString& name,
                              const QString& type,
                              const QVariantList& actorIds,
                              const QVariantList& propertyIds,
                              const QVariantList& aliases)
{
    QVariantMap out = basicFormState(name, aliases);
    const QVariantList normalizedActorIds = normalizeStrings(actorIds);
    QVariantList singleActorId;
    if (!normalizedActorIds.isEmpty()) {
        singleActorId.push_back(normalizedActorIds.first());
    }

    out.insert(ui::payload::keys::common::kType, type);
    out.insert(ui::payload::keys::state::kSelectedActorIds, singleActorId);
    out.insert(ui::payload::keys::state::kSelectedPropertyIds, normalizeStrings(propertyIds));
    return out;
}

QVariantList buildActorRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& actor : session.models().actors().actors()) {
        if (!actor) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(actor->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(actor->name));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(actor->name));
        QVariantList aliases;
        aliases.reserve(static_cast<int>(actor->aliases.size()));
        for (const auto& alias : actor->aliases) {
            QVariantMap aliasMap;
            aliasMap.insert(QStringLiteral("value"), QString::fromStdString(alias.value));
            aliasMap.insert(QStringLiteral("kind"), QString::fromStdString(alias.kind));
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
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(property->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(property->name));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(property->name));
        QVariantList aliases;
        aliases.reserve(static_cast<int>(property->aliases.size()));
        for (const auto& alias : property->aliases) {
            QVariantMap aliasMap;
            aliasMap.insert(QStringLiteral("value"), QString::fromStdString(alias.value));
            aliasMap.insert(QStringLiteral("kind"), QString::fromStdString(alias.kind));
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
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(contract->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(contract->name));
        row.insert(ui::payload::keys::common::kType, QString::fromStdString(contract->type));
        row.insert(ui::payload::keys::common::kDisplay, QString::fromStdString(contract->name));
        std::vector<std::string> aliases;
        aliases.reserve(contract->aliases.size());
        for (const auto& alias : contract->aliases) {
            aliases.push_back(alias.value);
        }
        row.insert(ui::payload::keys::contract::kAliases, payload::mapper::toVariantStringList(aliases));
        row.insert(ui::payload::keys::contract::kActorIds, payload::mapper::toQStringList(contract->actorIds));
        row.insert(ui::payload::keys::contract::kPropertyIds, payload::mapper::toQStringList(contract->propertyIds));
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
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(analysis->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(analysis->name));
        row.insert(ui::payload::keys::common::kType, QString::fromStdString(analysis->type));
        row.insert(ui::payload::keys::analysis::kConfig, QString::fromStdString(analysis->configJson));
        row.insert(ui::payload::keys::analysis::kFilter, QString::fromStdString(analysis->filterSpec));
        row.insert(ui::payload::keys::analysis::kAdjustments,
                   QString::fromStdString(core::application::AnalysisRequestComposer::serializeAdjustments(analysis->adjustments)));
        row.insert(ui::payload::keys::analysis::kExportFormat, QString::fromStdString(analysis->exportFormat));
        row.insert(ui::payload::keys::analysis::kIncludeCalcAdjustments, analysis->includeCalcAdjustments);
        row.insert(ui::payload::keys::analysis::kExportState, QString::fromStdString(analysis->exportStateJson));
        row.insert(ui::payload::keys::analysis::kSnapshotTransactions, QString::fromStdString(analysis->snapshotTransactionsJson));
        row.insert(ui::payload::keys::analysis::kCreatedAt, QString::fromStdString(analysis->createdAt));
        row.insert(ui::payload::keys::analysis::kUpdatedAt, QString::fromStdString(analysis->updatedAt));
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
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(annual->id));
        row.insert(ui::payload::keys::annual::kName, QString::fromStdString(annual->name));
        row.insert(ui::payload::keys::annual::kYear, annual->year);
        row.insert(ui::payload::keys::annual::kAnalysisIds, payload::mapper::toVariantStringList(annual->analysisIds));
        row.insert(ui::payload::keys::common::kDisplay,
                   annual->name.empty() ? QString::number(annual->year)
                                        : QString::fromStdString(annual->name));
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
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(statement->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(statement->name));
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
        if (QString::fromStdString(transaction->statementId) != statementId) continue;

        QVariantMap row;
        row.insert(ui::payload::keys::common::kId, QString::fromStdString(transaction->id));
        row.insert(ui::payload::keys::common::kName, QString::fromStdString(transaction->name));
        row.insert(ui::payload::keys::transaction::kBookingDate, QString::fromStdString(transaction->bookingDate));
        out.push_back(row);
    }
    return out;
}

} // namespace ui


