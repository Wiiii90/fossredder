#include "ui/state/SessionStore.h"

#include <algorithm>
#include <utility>

#include <QAbstractItemModel>
#include <QSet>

#include "ui/models/TransactionFilter.h"
#include "ui/state/SessionMutationState.h"

namespace ui {

namespace {

template <typename RefreshFn>
void bindSelectionRefresh(QAbstractItemModel& model, QObject* context, RefreshFn&& refresh)
{
    auto trigger = [refresh = std::forward<RefreshFn>(refresh)]() mutable {
        refresh();
    };

    QObject::connect(&model, &QAbstractItemModel::dataChanged, context, [trigger](const QModelIndex&, const QModelIndex&, const QVector<int>&) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::rowsInserted, context, [trigger](const QModelIndex&, int, int) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::rowsRemoved, context, [trigger](const QModelIndex&, int, int) mutable {
        trigger();
    });
    QObject::connect(&model, &QAbstractItemModel::modelReset, context, [trigger]() mutable {
        trigger();
    });
}

template <typename RefreshFn, typename... Models>
void bindSelectionRefreshes(QObject* context, RefreshFn&& refresh, Models&... models)
{
    (bindSelectionRefresh(models, context, refresh), ...);
}

template <typename RecomputeAllFn, typename RecomputeRangeFn>
void bindTransactionMetricSignals(TransactionList& transactions,
                                  QObject* context,
                                  RecomputeAllFn&& recomputeAll,
                                  RecomputeRangeFn&& recomputeRange)
{
    QObject::connect(&transactions, &QAbstractItemModel::rowsRemoved, context, [recomputeAll](const QModelIndex&, int, int) mutable {
        recomputeAll();
    });

    QObject::connect(&transactions, &QAbstractItemModel::rowsInserted, context, [recomputeRange](const QModelIndex&, int first, int last) mutable {
        recomputeRange(first, last);
    });

    QObject::connect(&transactions, &QAbstractItemModel::dataChanged, context, [recomputeAll, recomputeRange](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) mutable {
        if (roles.isEmpty()) {
            recomputeAll();
            return;
        }

        bool propertyIdsChanged = false;
        bool amountOrAllocChanged = false;
        for (int role : roles) {
            if (role == TransactionList::PropertyIdsRole) propertyIdsChanged = true;
            if (role == TransactionList::AmountRole || role == TransactionList::AllocatableRole) amountOrAllocChanged = true;
        }

        if (propertyIdsChanged) {
            recomputeAll();
            return;
        }

        if (!amountOrAllocChanged) return;

        const int first = topLeft.isValid() ? topLeft.row() : -1;
        const int last = bottomRight.isValid() ? bottomRight.row() : first;
        recomputeRange(first, last);
    });

    QObject::connect(&transactions, &QAbstractItemModel::modelReset, context, [recomputeAll]() mutable {
        recomputeAll();
    });
}

template <typename ClearCacheFn, typename NotifyFn>
void bindAnalysisMetricSignals(AnalysisList& analyses,
                               QObject* context,
                               ClearCacheFn&& clearCache,
                               NotifyFn&& notifyAll)
{
    QObject::connect(&analyses, &QAbstractItemModel::modelReset, context, [clearCache, notifyAll]() mutable {
        clearCache();
        notifyAll();
    });
}

}

SessionStore::SessionStore(QObject* parent)
    : QObject(parent)
    , models_(this)
{
    bindModelSignals();
}

void SessionStore::bindModelSignals()
{
    bindSelectionRefreshes(this,
                           [this]() {
                               emit selectionRefreshRequested();
                           },
                           models_.actors(),
                           models_.properties(),
                           models_.contracts(),
                           models_.statements(),
                           models_.transactions(),
                           models_.analyses(),
                           models_.annuals());

    bindTransactionMetricSignals(models_.transactions(),
                                 this,
                                 [this]() {
                                     recomputeAllMetrics();
                                 },
                                 [this](int first, int last) {
                                     recomputeMetricsForRows(first, last);
                                 });

    bindAnalysisMetricSignals(models_.analyses(),
                              this,
                              [this]() {
                                  metrics_.clearCache();
                              },
                              [this]() {
                                  notifyTransactionSumsForAllProperties();
                              });
}

void SessionStore::recomputeAllMetrics()
{
    metrics_.recomputeAll(models_.properties(),
                          models_.transactions(),
                          models_.contracts(),
                          [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
}

void SessionStore::recomputeMetricsForRows(int firstRow, int lastRow)
{
    if (firstRow < 0 || lastRow < firstRow) {
        recomputeAllMetrics();
        return;
    }

    QSet<QString> affectedPropertyIds;
    const auto& rows = models_.transactions().transactions();
    const int safeLast = std::min(lastRow, static_cast<int>(rows.size()) - 1);
    for (int row = firstRow; row <= safeLast; ++row) {
        const auto& transaction = rows[static_cast<size_t>(row)];
        if (!transaction) continue;
        for (const auto& propertyId : transaction->propertyIds) affectedPropertyIds.insert(QString::fromStdString(propertyId));
    }

    if (affectedPropertyIds.isEmpty()) {
        recomputeAllMetrics();
        return;
    }

    for (const auto& propertyId : affectedPropertyIds) {
        metrics_.recomputeProperty(propertyId,
                                   models_.transactions(),
                                   models_.contracts(),
                                   [this](const QString& id) {
            emit transactionSumsUpdated(id);
        });
    }
}

void SessionStore::notifyTransactionSumsForAllProperties()
{
    for (const auto& property : models_.properties().properties()) {
        if (!property) continue;
        emit transactionSumsUpdated(QString::fromStdString(property->id));
    }
}

void SessionStore::loadFromState(const AppState& state)
{
    filters_.clear();
    models_.loadFromState(state);
    propertyNames_.rebuild(models_.properties());
    recomputeAllMetrics();
    emit selectionRefreshRequested();
}

TransactionFilter* SessionStore::statementTransactions(const QString& statementId, QObject* parent)
{
    return filters_.statementTransactions(statementId, models_.transactions(), parent);
}

TransactionFilter* SessionStore::propertyTransactions(const QString& propertyId, QObject* parent)
{
    return filters_.propertyTransactions(propertyId, models_.transactions(), parent);
}

QStringList SessionStore::propertyContractTypes(const QString& propertyId) const
{
    return metrics_.propertyContractTypes(propertyId, models_.transactions(), models_.contracts());
}

QVariantMap SessionStore::propertyTransactionSums(const QString& propertyId, const QString& contractType) const
{
    return metrics_.propertyTransactionSums(propertyId, contractType, models_.transactions(), models_.contracts());
}

QString SessionStore::propertyName(const QString& id) const
{
    return propertyNames_.name(id);
}

void SessionStore::applyDeletionImpact(const DeletionImpact& impact)
{
    SessionMutationState::applyDeletionImpact(impact,
                                              models_,
                                              filters_,
                                              metrics_,
                                              propertyNames_);
    models_.refreshContractTypes();
    emit selectionRefreshRequested();
}

void SessionStore::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    SessionMutationState::setTransactionPropertyIdsImmediate(txId,
                                                             propertyIds,
                                                             models_,
                                                             metrics_,
                                                             [this](const QString& propertyId) {
        emit transactionSumsUpdated(propertyId);
    });
    emit selectionRefreshRequested();
}

}
