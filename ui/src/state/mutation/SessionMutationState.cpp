/**
 * @file ui/src/state/SessionMutationState.cpp
 * @brief Implementation of the UI SessionMutationState component.
 */

#include "ui/state/mutation/SessionMutationState.h"

#include <algorithm>
#include <QModelIndex>
#include <QSet>

#include "ui/state/filters/FilterState.h"
#include "ui/state/session/WorkspaceSessionModels.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {

namespace {

template <typename IdRange> QSet<QString> toQStringSet(const IdRange &ids) {
  QSet<QString> values;
  values.reserve(static_cast<qsizetype>(ids.size()));
  for (const auto &id : ids)
    values.insert(QString::fromStdString(id));
  return values;
}

std::vector<std::string> toStdStringVector(const QStringList &ids) {
  std::vector<std::string> values;
  values.reserve(static_cast<size_t>(ids.size()));
  for (const auto &id : ids)
    values.push_back(strings::toStdString(id));
  return values;
}

template <typename IdRange, typename Model, typename Cleanup>
void removeDeletedIds(const IdRange &ids, Model &model, Cleanup &&cleanup) {
  for (const auto &rawId : ids) {
    const QString id = QString::fromStdString(rawId);
    const int row = model.findRowById(id);
    if (row >= 0)
      model.removeAt(row);
    cleanup(id);
  }
}

void assignTransactionPropertyIds(Transaction &transaction,
                                  const QStringList &propertyIds) {
  transaction.setPropertyIds(toStdStringVector(propertyIds));
}

} // namespace

QVariantList SessionMutationState::normalizeStrings(const QVariantList& values)
{
    QVariantList out;
    out.reserve(values.size());
    for (const auto& value : values) {
        out.push_back(value.toString());
    }
    return out;
}

QVariantList SessionMutationState::addUniqueTrimmed(const QVariantList& values, const QString& value)
{
    const QString trimmed = value.trimmed();
    QVariantList out = normalizeStrings(values);
    if (trimmed.isEmpty()) {
        return out;
    }

    for (const auto& current : out) {
        if (current.toString() == trimmed) {
            return out;
        }
    }

    out.push_back(trimmed);
    return out;
}

QVariantList SessionMutationState::removeAt(const QVariantList& values, int index)
{
    QVariantList out = normalizeStrings(values);
    if (index < 0 || index >= out.size()) {
        return out;
    }
    out.removeAt(index);
    return out;
}

QVariantList SessionMutationState::removeString(const QVariantList& values, const QString& value)
{
    QVariantList out;
    out.reserve(values.size());
    for (const auto& item : values) {
        const QString current = item.toString();
        if (current != value) {
            out.push_back(current);
        }
    }
    return out;
}

QVariantList SessionMutationState::insertAt(const QVariantList& values, int index, const QVariant& value)
{
    QVariantList out = values;
    const int size = static_cast<int>(out.size());
    const int bounded = std::clamp(index, 0, size);
    out.insert(bounded, value);
    return out;
}

void SessionMutationState::applyDeletionImpact(
    const DeletionImpact &impact, SessionModels &models, FilterState &filters) {
  removeDeletedIds(impact.deletedTransactionIds, models.transactions(),
                   [](const QString &) {});

  removeDeletedIds(
      impact.deletedStatementIds, models.statements(),
      [&filters](const QString &id) { filters.removeStatement(id); });

  removeDeletedIds(impact.deletedPropertyIds, models.properties(),
                   [&filters](const QString &id) { filters.removeProperty(id); });

  removeDeletedIds(impact.deletedActorIds, models.actors(),
                   [](const QString &) {});
  removeDeletedIds(impact.deletedContractIds, models.contracts(),
                   [](const QString &) {});

  removeDeletedIds(impact.deletedAnalysisIds, models.analyses(),
                   [](const QString &) {});
  removeDeletedIds(impact.deletedAnnualIds, models.annuals(),
                   [](const QString &) {});
}

void SessionMutationState::setTransactionPropertyIdsImmediate(
    const QString &transactionId, const QStringList &propertyIds,
    SessionModels &models) {
  if (transactionId.isEmpty())
    return;
  const int row = models.transactions().findRowById(transactionId);
  if (row < 0)
    return;

  auto current =
      models.transactions().transactions().at(static_cast<size_t>(row));
  if (!current)
    return;

  QSet<QString> oldSet = toQStringSet(current->propertyIds());
  const QSet<QString> newSet(propertyIds.begin(), propertyIds.end());

  if (oldSet == newSet)
    return;

  assignTransactionPropertyIds(*current, propertyIds);

  const QModelIndex modelIndex = models.transactions().index(row);
  emit models.transactions().dataChanged(
      modelIndex, modelIndex, {ui::TransactionList::PropertyIdsRole});
}

QVariantMap SessionMutationState::emptyTransactionDraft()
{
    QVariantMap tx;
    tx.insert(QStringLiteral("id"), QString());
    tx.insert(QStringLiteral("name"), QString());
    tx.insert(QStringLiteral("bookingDate"), QString());
    tx.insert(QStringLiteral("valuta"), QString());
    tx.insert(QStringLiteral("amount"), 0.0);
    tx.insert(QStringLiteral("description"), QString());
    tx.insert(QStringLiteral("status"), 0);
    tx.insert(QStringLiteral("actorId"), QString());
    tx.insert(QStringLiteral("propertyIds"), QVariantList());
    tx.insert(QStringLiteral("allocatable"), false);
    tx.insert(QStringLiteral("contractId"), QString());
    tx.insert(QStringLiteral("statementId"), QString());
    return tx;
}

QVariantMap SessionMutationState::normalizeTransactionDraft(const QVariantMap& tx)
{
    QVariantMap out = emptyTransactionDraft();
    for (auto it = tx.constBegin(); it != tx.constEnd(); ++it) {
        out.insert(it.key(), it.value());
    }

    out.insert(QStringLiteral("amount"), out.value(QStringLiteral("amount")).toDouble());
    out.insert(QStringLiteral("status"), out.value(QStringLiteral("status")).toInt());
    out.insert(QStringLiteral("propertyIds"), normalizeStrings(out.value(QStringLiteral("propertyIds")).toList()));
    out.insert(QStringLiteral("actorId"), out.value(QStringLiteral("actorId")).toString());
    out.insert(QStringLiteral("contractId"), out.value(QStringLiteral("contractId")).toString());
    out.insert(QStringLiteral("name"), out.value(QStringLiteral("name")).toString());
    out.insert(QStringLiteral("bookingDate"), out.value(QStringLiteral("bookingDate")).toString());
    out.insert(QStringLiteral("valuta"), out.value(QStringLiteral("valuta")).toString());
    out.insert(QStringLiteral("description"), out.value(QStringLiteral("description")).toString());
    out.insert(QStringLiteral("statementId"), out.value(QStringLiteral("statementId")).toString());
    out.insert(QStringLiteral("allocatable"), out.value(QStringLiteral("allocatable")).toBool());
    return out;
}

QVariantList SessionMutationState::normalizeTransactionDrafts(const QVariantList& values)
{
    QVariantList out;
    out.reserve(values.size());
    for (const auto& value : values) {
        out.push_back(normalizeTransactionDraft(value.toMap()));
    }
    return out;
}

bool SessionMutationState::transactionDraftHasContent(const QVariantMap& tx)
{
    const QVariantMap normalized = normalizeTransactionDraft(tx);
    return !normalized.value(QStringLiteral("name")).toString().isEmpty()
        || !normalized.value(QStringLiteral("bookingDate")).toString().isEmpty()
        || !normalized.value(QStringLiteral("valuta")).toString().isEmpty()
        || normalized.value(QStringLiteral("amount")).toDouble() != 0.0
        || !normalized.value(QStringLiteral("actorId")).toString().isEmpty()
        || !normalized.value(QStringLiteral("propertyIds")).toList().isEmpty();
}

QVariantMap SessionMutationState::createDraftListState(const QVariantList& drafts,
                                                      int currentIndex,
                                                      const QVariantMap& emptyDraft)
{
    QVariantList normalizedDrafts = normalizeTransactionDrafts(drafts);
    if (normalizedDrafts.isEmpty()) {
        normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
    }

    const int lastIndex = static_cast<int>(normalizedDrafts.size()) - 1;
    const int index = std::clamp(currentIndex, 0, lastIndex);
    QVariantMap out;
    out.insert(QStringLiteral("drafts"), normalizedDrafts);
    out.insert(QStringLiteral("index"), index);
    return out;
}

QVariantMap SessionMutationState::insertDraftAfterCurrent(const QVariantList& drafts,
                                                         int currentIndex,
                                                         const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
    const int index = base.value(QStringLiteral("index")).toInt();
    const int insertIndex = std::clamp(index + 1, 0, static_cast<int>(normalizedDrafts.size()));
    normalizedDrafts.insert(insertIndex, normalizeTransactionDraft(emptyDraft));

    QVariantMap out;
    out.insert(QStringLiteral("drafts"), normalizedDrafts);
    out.insert(QStringLiteral("index"), insertIndex);
    return out;
}

QVariantMap SessionMutationState::removeDraftAt(const QVariantList& drafts,
                                                int currentIndex,
                                                const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
    int index = base.value(QStringLiteral("index")).toInt();
    if (index < 0 || index >= normalizedDrafts.size()) {
        index = 0;
    }

    normalizedDrafts.removeAt(index);
    if (normalizedDrafts.isEmpty()) {
        normalizedDrafts.push_back(normalizeTransactionDraft(emptyDraft));
        index = 0;
    } else {
        const int lastIndex = static_cast<int>(normalizedDrafts.size()) - 1;
        index = std::clamp(index, 0, lastIndex);
    }

    QVariantMap out;
    out.insert(QStringLiteral("drafts"), normalizedDrafts);
    out.insert(QStringLiteral("index"), index);
    return out;
}

QVariantMap SessionMutationState::setCurrentDraft(const QVariantList& drafts,
                                                  int currentIndex,
                                                  const QVariantMap& draft,
                                                  const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
    const int index = base.value(QStringLiteral("index")).toInt();
    normalizedDrafts[index] = normalizeTransactionDraft(draft);

    QVariantMap out;
    out.insert(QStringLiteral("drafts"), normalizedDrafts);
    out.insert(QStringLiteral("index"), index);
    return out;
}

QVariantMap SessionMutationState::currentDraftState(const QVariantList& drafts,
                                                    int currentIndex,
                                                    const QVariantMap& emptyDraft)
{
    const QVariantMap base = createDraftListState(drafts, currentIndex, emptyDraft);
    const QVariantList normalizedDrafts = base.value(QStringLiteral("drafts")).toList();
    const int index = base.value(QStringLiteral("index")).toInt();

    QVariantMap out;
    out.insert(QStringLiteral("drafts"), normalizedDrafts);
    out.insert(QStringLiteral("index"), index);
    out.insert(QStringLiteral("draft"), normalizeTransactionDraft(normalizedDrafts.at(index).toMap()));
    return out;
}

} // namespace ui

