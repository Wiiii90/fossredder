/**
 * @file ui/include/ui/state/StateFacadeProjection.h
 * @brief Declares helpers that project session store data into QML-friendly rows.
 */

#pragma once

#include <QVariantList>
#include <QVariantMap>

#include <QString>

namespace ui {

class SessionStore;

QVariantList buildStatementTransactionIds(const SessionStore& session, const QString& statementId);
QVariantList buildActorRows(const SessionStore& session);
QVariantList buildPropertyRows(const SessionStore& session);
QVariantList buildContractRows(const SessionStore& session);
QVariantList buildAnalysisRows(const SessionStore& session);
QVariantList buildStatementRows(const SessionStore& session);
QVariantList buildStatementTransactionRows(const SessionStore& session, const QString& statementId);

// Generic list and index helpers
QVariantList normalizeStrings(const QVariantList& values);
QVariantList addUniqueTrimmed(const QVariantList& values, const QString& value);
QVariantList removeAt(const QVariantList& values, int index);
QVariantList removeString(const QVariantList& values, const QString& value);
QVariantList insertAt(const QVariantList& values, int index, const QVariant& value);
QVariantList pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds);
int indexOfId(const QVariantList& rows, const QString& id);
int indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value);
int indexOfString(const QVariantList& values, const QString& value);
int normalizedIndex(int index, int count);
int wrappedIndex(int index, int count);
QString wrappedIdAt(const QVariantList& rows, int index);
QString navigatedId(const QVariantList& rows,
                   const QString& currentId,
                   int delta,
                   int fallbackIndex = 0);

// Row projection helpers
QVariantList displayRowsWithEmpty(const QVariantList& rows,
                                  const QString& emptyDisplay,
                                  const QString& displayKey = QStringLiteral("display"));
QVariantList rowIds(const QVariantList& rows, const QString& idKey = QStringLiteral("id"));
QVariantList orderedRowsByIds(const QVariantList& rows,
                              const QVariantList& orderIds,
                              const QString& idKey = QStringLiteral("id"));
QVariantMap mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value);

// Draft state helpers
QVariantMap emptyTransactionDraft();
QVariantMap normalizeTransactionDraft(const QVariantMap& tx);
QVariantList normalizeTransactionDrafts(const QVariantList& values);
bool transactionDraftHasContent(const QVariantMap& tx);
QVariantMap createDraftListState(const QVariantList& drafts,
                                 int currentIndex,
                                 const QVariantMap& emptyDraft);
QVariantMap insertDraftAfterCurrent(const QVariantList& drafts,
                                    int currentIndex,
                                    const QVariantMap& emptyDraft);
QVariantMap removeDraftAt(const QVariantList& drafts,
                          int currentIndex,
                          const QVariantMap& emptyDraft);
QVariantMap setCurrentDraft(const QVariantList& drafts,
                            int currentIndex,
                            const QVariantMap& draft,
                            const QVariantMap& emptyDraft);
QVariantMap currentDraftState(const QVariantList& drafts,
                              int currentIndex,
                              const QVariantMap& emptyDraft);

// Selection and ordering state helpers
QVariantMap resolveSelectionState(const QVariantList& rows,
                                  int currentIndex,
                                  const QString& selectedId,
                                  const QString& idKey = QStringLiteral("id"));
QVariantList orderWithInsertedId(const QVariantList& currentOrder,
                                 const QVariantList& availableIds,
                                 const QString& insertedId,
                                 int insertAfterIndex);
QVariantMap orderedRowsState(const QVariantList& rows,
                             const QVariantList& preferredOrder,
                             const QString& idKey = QStringLiteral("id"));
QVariantMap orderedSelectionState(const QVariantList& rows,
                                  const QVariantList& preferredOrder,
                                  int currentIndex,
                                  const QString& selectedId,
                                  const QString& idKey = QStringLiteral("id"));
QVariantMap navigateSelectionState(const QVariantList& rows,
                                   int currentIndex,
                                   const QString& selectedId,
                                   int delta,
                                   int fallbackIndex = 0,
                                   const QString& idKey = QStringLiteral("id"));
QVariantMap deleteReselectionState(const QVariantList& rows,
                                   const QVariantList& preferredOrder,
                                   int currentIndex,
                                   const QString& removedId,
                                   const QString& idKey = QStringLiteral("id"));
QString deleteNextSelectionId(const QVariantList& rows,
                              const QString& removedId,
                              int fallbackIndex = 0,
                              const QString& idKey = QStringLiteral("id"));

// Form state helpers
QVariantMap basicFormState(const QString& name,
                           const QVariantList& aliases,
                           const QVariantList& selectedIds = QVariantList());
QVariantMap contractFormState(const QString& name,
                              const QString& type,
                              const QVariantList& actorIds,
                              const QVariantList& propertyIds,
                              const QVariantList& aliases);

} // namespace ui
