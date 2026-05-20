/**
 * @file ui/include/ui/state/SessionMutationState.h
 * @brief Declarations for the UI SessionMutationState component.
 */

#pragma once

#include <functional>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QStringList>

#include "core/application/storage/DeletionImpact.h"
#include "core/domain/entities/Transaction.h"

namespace ui {

class SessionModels;
class FilterState;

class SessionMutationState {
public:
    static QVariantList normalizeStrings(const QVariantList& values);
    static QVariantMap transactionDraft(const QVariantMap& draft,
                                        const QVariantList& contractRows,
                                        const QVariantMap& changes);
    static QVariantList addUniqueTrimmed(const QVariantList& values, const QString& value);
    static QVariantList removeAt(const QVariantList& values, int index);
    static QVariantList removeString(const QVariantList& values, const QString& value);
    static QVariantList insertAt(const QVariantList& values, int index, const QVariant& value);
    static QVariantMap emptyTransactionDraft();
    static QVariantMap normalizeTransactionDraft(const QVariantMap& tx);
    static QVariantList normalizeTransactionDrafts(const QVariantList& values);
    static bool transactionDraftHasContent(const QVariantMap& tx);
    static QVariantMap createDraftListState(const QVariantList& drafts,
                                            int currentIndex,
                                            const QVariantMap& emptyDraft);
    static QVariantMap insertDraftAfterCurrent(const QVariantList& drafts,
                                               int currentIndex,
                                               const QVariantMap& emptyDraft);
    static QVariantMap removeDraftAt(const QVariantList& drafts,
                                     int currentIndex,
                                     const QVariantMap& emptyDraft);
    static QVariantMap setCurrentDraft(const QVariantList& drafts,
                                       int currentIndex,
                                       const QVariantMap& draft,
                                       const QVariantMap& emptyDraft);
    static QVariantMap currentDraftState(const QVariantList& drafts,
                                         int currentIndex,
                                         const QVariantMap& emptyDraft);

    static void applyDeletionImpact(const core::domain::DeletionImpact& impact,
                                    SessionModels& models,
                                    FilterState& filters);

    static void setTransactionPropertyIdsImmediate(const QString& transactionId,
                                                   const QStringList& propertyIds,
                                                    SessionModels& models);
};

}
