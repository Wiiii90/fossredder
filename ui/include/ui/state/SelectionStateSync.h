/**
 * @file ui/include/ui/state/SelectionStateSync.h
 * @brief Declares helpers for binding and comparing UI selection state refreshes.
 */

#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

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

int selectionIndexOfId(const QVariantList& rows, const QString& id, const QString& idKey = QStringLiteral("id"));
int selectionIndexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value);
int selectionIndexOfString(const QVariantList& values, const QString& value);
int normalizedSelectionIndex(int index, int count);
int wrappedSelectionIndex(int index, int count);
QString wrappedSelectionIdAt(const QVariantList& rows, int index, const QString& idKey = QStringLiteral("id"));
QString navigatedSelectionId(const QVariantList& rows,
                             const QString& currentId,
                             int delta,
                             int fallbackIndex = 0,
                             const QString& idKey = QStringLiteral("id"));
QVariantList pruneAndAppendMissingIds(const QVariantList& preferredIds, const QVariantList& availableIds);
QVariantList selectionRowIds(const QVariantList& rows, const QString& idKey = QStringLiteral("id"));
QVariantList orderedRowsBySelectionIds(const QVariantList& rows,
                                       const QVariantList& orderIds,
                                       const QString& idKey = QStringLiteral("id"));
QVariantMap resolveSelectionRowState(const QVariantList& rows,
                                     int currentIndex,
                                     const QString& selectedId,
                                     const QString& idKey = QStringLiteral("id"));
QVariantList orderWithInsertedSelectionId(const QVariantList& currentOrder,
                                          const QVariantList& availableIds,
                                          const QString& insertedId,
                                          int insertAfterIndex);
QVariantMap orderedSelectionRowsState(const QVariantList& rows,
                                      const QVariantList& preferredOrder,
                                      const QString& idKey = QStringLiteral("id"));
QVariantMap orderedSelectionStateForRows(const QVariantList& rows,
                                         const QVariantList& preferredOrder,
                                         int currentIndex,
                                         const QString& selectedId,
                                         const QString& idKey = QStringLiteral("id"));
QVariantMap navigateSelectionDeltaState(const QVariantList& rows,
                                        int currentIndex,
                                        const QString& selectedId,
                                        int delta,
                                        int fallbackIndex = 0,
                                        const QString& idKey = QStringLiteral("id"));
QVariantMap deleteReselectionStateForRows(const QVariantList& rows,
                                          const QVariantList& preferredOrder,
                                          int currentIndex,
                                          const QString& removedId,
                                          const QString& idKey = QStringLiteral("id"));
QString deleteNextSelectionIdForRows(const QVariantList& rows,
                                     const QString& removedId,
                                     int fallbackIndex = 0,
                                     const QString& idKey = QStringLiteral("id"));

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
