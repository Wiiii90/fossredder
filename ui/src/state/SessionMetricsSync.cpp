/**
 * @file ui/src/state/SessionMetricsSync.cpp
 * @brief Implements helpers for session metric recomputation and model signal binding.
 */

#include "ui/state/SessionMetricsSync.h"

#include <algorithm>

#include <QAbstractItemModel>
#include <QSet>

#include "ui/models/AnalysisList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/TransactionList.h"
#include "ui/state/MetricsState.h"
#include "ui/state/SessionModels.h"

namespace ui {

void bindSessionMetricSignals(SessionModels& models,
                              QObject* context,
                              const std::function<void()>& recomputeAll,
                              const std::function<void(int, int)>& recomputeRange,
                              const std::function<void()>& clearAnalysisMetrics,
                              const std::function<void()>& notifyAllProperties)
{
    auto& transactions = models.transactions();
    QObject::connect(&transactions, &QAbstractItemModel::rowsRemoved, context, [recomputeAll](const QModelIndex&, int, int) {
        recomputeAll();
    });

    QObject::connect(&transactions, &QAbstractItemModel::rowsInserted, context, [recomputeRange](const QModelIndex&, int first, int last) {
        recomputeRange(first, last);
    });

    QObject::connect(&transactions, &QAbstractItemModel::dataChanged, context, [recomputeAll, recomputeRange](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        if (roles.isEmpty()) {
            recomputeAll();
            return;
        }

        bool propertyIdsChanged = false;
        bool amountOrAllocChanged = false;
        for (const int role : roles) {
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

    QObject::connect(&transactions, &QAbstractItemModel::modelReset, context, [recomputeAll]() {
        recomputeAll();
    });

    auto& analyses = models.analyses();
    QObject::connect(&analyses, &QAbstractItemModel::modelReset, context, [clearAnalysisMetrics, notifyAllProperties]() {
        clearAnalysisMetrics();
        notifyAllProperties();
    });
}

void recomputeAllSessionMetrics(const SessionModels& models,
                                MetricsState& metrics,
                                const PropertyChangedCallback& notifyChanged)
{
    metrics.recomputeAll(models.properties(),
                         models.transactions(),
                         models.contracts(),
                         notifyChanged);
}

void recomputeSessionMetricsForRows(int firstRow,
                                    int lastRow,
                                    const SessionModels& models,
                                    MetricsState& metrics,
                                    const PropertyChangedCallback& notifyChanged)
{
    if (firstRow < 0 || lastRow < firstRow) {
        recomputeAllSessionMetrics(models, metrics, notifyChanged);
        return;
    }

    QSet<QString> affectedPropertyIds;
    const auto& rows = models.transactions().transactions();
    const int safeLast = std::min(lastRow, static_cast<int>(rows.size()) - 1);
    for (int row = firstRow; row <= safeLast; ++row) {
        const auto& transaction = rows[static_cast<std::size_t>(row)];
        if (!transaction) continue;
        for (const auto& propertyId : transaction->propertyIds) {
            affectedPropertyIds.insert(QString::fromStdString(propertyId));
        }
    }

    if (affectedPropertyIds.isEmpty()) {
        recomputeAllSessionMetrics(models, metrics, notifyChanged);
        return;
    }

    for (const auto& propertyId : affectedPropertyIds) {
        metrics.recomputeProperty(propertyId,
                                  models.transactions(),
                                  models.contracts(),
                                  notifyChanged);
    }
}

void notifySessionMetricsForAllProperties(const SessionModels& models,
                                          const PropertyChangedCallback& notifyChanged)
{
    for (const auto& property : models.properties().properties()) {
        if (!property) continue;
        notifyChanged(QString::fromStdString(property->id));
    }
}

} // namespace ui
