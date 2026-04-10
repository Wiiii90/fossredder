/**
 * @file ui/src/state/SessionMutationState.cpp
 * @brief Implementation of the UI SessionMutationState component.
 */

#include "ui/state/SessionMutationState.h"

#include <QModelIndex>
#include <QSet>

#include "ui/state/FilterState.h"
#include "ui/state/MetricsState.h"
#include "ui/state/PropertyNameCatalog.h"
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

void recomputeMetricsForPropertyIds(
    const QSet<QString> &propertyIds, SessionModels &models,
    MetricsState &metrics,
    const SessionMutationState::TransactionSumsNotifier
        &notifyTransactionSums) {
  for (const auto &propertyId : propertyIds) {
    metrics.recomputeProperty(propertyId, models.transactions(),
                              models.contracts(), notifyTransactionSums);
  }
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

void SessionMutationState::applyDeletionImpact(
    const DeletionImpact &impact, SessionModels &models, FilterState &filters,
    MetricsState &metrics, PropertyNameCatalog &propertyNames) {
  removeDeletedIds(impact.deletedTransactionIds, models.transactions(),
                   [](const QString &) {});

  removeDeletedIds(
      impact.deletedStatementIds, models.statements(),
      [&filters](const QString &id) { filters.removeStatement(id); });

  removeDeletedIds(impact.deletedPropertyIds, models.properties(),
                   [&filters, &metrics, &propertyNames](const QString &id) {
                     propertyNames.remove(id);
                     metrics.removePropertyCache(id);
                     filters.removeProperty(id);
                   });

  removeDeletedIds(impact.deletedActorIds, models.actors(),
                   [](const QString &) {});
  removeDeletedIds(impact.deletedContractIds, models.contracts(),
                   [](const QString &) {});
}

void SessionMutationState::setTransactionPropertyIdsImmediate(
    const QString &transactionId, const QStringList &propertyIds,
    SessionModels &models, MetricsState &metrics,
    const TransactionSumsNotifier &notifyTransactionSums) {
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

  oldSet.unite(newSet);
  recomputeMetricsForPropertyIds(oldSet, models, metrics,
                                 notifyTransactionSums);
}

} // namespace ui
