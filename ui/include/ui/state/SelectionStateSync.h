/**
 * @file ui/include/ui/state/SelectionStateSync.h
 * @brief Declares helpers for binding and comparing UI selection state refreshes.
 */

#pragma once

#include <QAbstractItemModel>
#include <QString>

#include <utility>

#include "ui/state/SelectionState.h"

namespace ui {

class SessionSelection;

struct SelectionIdsSnapshot {
    QString actorId;
    QString propertyId;
    QString contractId;
    QString statementId;
    QString transactionId;
    QString analysisId;
    QString annualId;
};

SelectionIdsSnapshot captureSelectionIds(const SelectionState& selection);
void emitSelectionChanges(SessionSelection& selection, const SelectionIdsSnapshot& before);

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

} // namespace ui
