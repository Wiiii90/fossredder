/**
 * @file ui/src/state/SessionMutationState.cpp
 * @brief Implementation of the UI SessionMutationState component.
 */

#include "ui/state/SessionMutationState.h"

#include <algorithm>
#include <QModelIndex>
#include <QSet>

#include "ui/state/FilterState.h"
#include "ui/state/SessionModels.h"
#include "ui/util/StringConversions.h"

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
  transaction.propertyIds = toStdStringVector(propertyIds);
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
    const int bounded = std::max(0, std::min(index, size));
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

  QSet<QString> oldSet = toQStringSet(current->propertyIds);
  const QSet<QString> newSet(propertyIds.begin(), propertyIds.end());

  if (oldSet == newSet)
    return;

  assignTransactionPropertyIds(*current, propertyIds);

  const QModelIndex modelIndex = models.transactions().index(row);
  emit models.transactions().dataChanged(
      modelIndex, modelIndex, {ui::TransactionList::PropertyIdsRole});
}

} // namespace ui
