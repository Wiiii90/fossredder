/**
 * @file ui/include/ui/state/session/WorkspaceSessionState.h
 * @brief Declares the UI session store that owns models, filters and derived
 * metrics.
 */

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "core/application/storage/DeletionImpact.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "ui/state/filters/FilterState.h"
#include "ui/state/session/WorkspaceSessionModels.h"
#include "ui/viewmodels/booking/TransactionFilterModel.h"

namespace ui {

/**
 * @brief Owns the UI-facing session models and derived lookup/filter state.
 */
class SessionState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(SessionState)
  QML_UNCREATABLE("SessionState is exposed by the application context")
  Q_PROPERTY(int dataRevision READ dataRevision NOTIFY dataRevisionChanged)

public:
  /** @brief Creates the session store and connects derived-state recomputation.
   */
  explicit SessionState(QObject *parent = nullptr);

  SessionModels &models() noexcept { return models_; }
  const SessionModels &models() const noexcept { return models_; }

  /** @brief Replaces all UI model data from a domain application state
   * snapshot. */
  void loadFromState(const core::domain::catalog::WorkspaceCatalog &state);
  /** @brief Returns a live filter over transactions for a statement. */
  TransactionFilter *statementTransactions(const QString &statementId);
  /** @brief Returns a live filter over transactions for a property. */
  TransactionFilter *propertyTransactions(const QString &propertyId);
  /** @brief Applies domain deletion effects to session models. */
  void applyDeletionImpact(const core::domain::DeletionImpact &impact);
  /** @brief Applies an immediate property reassignment for a single
   * transaction. */
  void setTransactionPropertyIdsImmediate(const QString &txId,
                                          const QStringList &propertyIds);
  int dataRevision() const noexcept { return dataRevision_; }

  Q_INVOKABLE QVariantList normalizeStrings(const QVariantList &values) const;
  Q_INVOKABLE QString normalizedStringListKey(const QVariantList &values) const;
  Q_INVOKABLE QVariantMap transactionDraft(const QVariantMap &draft,
                                           const QVariantList &contractRows,
                                           const QVariantMap &changes) const;
  Q_INVOKABLE QVariantList addUniqueTrimmed(const QVariantList &values,
                                            const QString &value) const;
  Q_INVOKABLE QVariantList removeAt(const QVariantList &values,
                                    int index) const;
  Q_INVOKABLE QVariantList removeString(const QVariantList &values,
                                        const QString &value) const;
  Q_INVOKABLE bool formStateChanged(const QString &savedName,
                                    const QVariantList &savedAliases,
                                    const QVariantList &savedContractIds,
                                    const QString &currentName,
                                    const QVariantList &aliases,
                                    const QVariantList &contractIds) const;
  Q_INVOKABLE QVariantList insertAt(const QVariantList &values, int index,
                                    const QVariant &value) const;
  Q_INVOKABLE QVariantList pruneAndAppendMissing(
      const QVariantList &preferredIds, const QVariantList &availableIds) const;
  Q_INVOKABLE int indexOfId(const QVariantList &rows, const QString &id) const;
  Q_INVOKABLE int indexOfKeyValue(const QVariantList &rows, const QString &key,
                                  const QVariant &value) const;
  Q_INVOKABLE int indexOfString(const QVariantList &values,
                                const QString &value) const;
  Q_INVOKABLE int normalizedIndex(int index, int count) const;
  Q_INVOKABLE int wrappedIndex(int index, int count) const;
  Q_INVOKABLE QString wrappedIdAt(const QVariantList &rows, int index) const;
  Q_INVOKABLE QString navigatedId(const QVariantList &rows,
                                  const QString &currentId, int delta,
                                  int defaultIndex = 0) const;
  Q_INVOKABLE QString navigatedSelectionId(
      const QVariantList &rows, const QString &currentId, int delta,
      int defaultIndex = 0, const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantList displayRowsWithEmpty(
      const QVariantList &rows, const QString &emptyDisplay,
      const QString &displayKey = QStringLiteral("display")) const;
  Q_INVOKABLE QVariantList
  rowIds(const QVariantList &rows,
         const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE bool rowHasId(const QVariantList &rows, const QString &id,
                            const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QString rememberedOrFirstRowId(
      const QVariantList &rows, const QVariantMap &rememberedIds,
      const QString &ownerId, const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantList
  orderedRowsByIds(const QVariantList &rows, const QVariantList &orderIds,
                   const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantMap mapWithKeyValue(const QVariantMap &base,
                                          const QString &key,
                                          const QVariant &value) const;
  Q_INVOKABLE QVariantMap emptyTransactionDraft() const;
  Q_INVOKABLE QVariantMap
  normalizeTransactionDraft(const QVariantMap &tx) const;
  Q_INVOKABLE QString transactionDraftSnapshot(const QVariantMap &tx) const;
  Q_INVOKABLE bool bookingEditStateChanged(const QString &savedStatementName,
                                           const QString &savedTransactionJson,
                                           const QString &currentStatementName,
                                           const QVariantMap &transaction) const;
  Q_INVOKABLE QVariantList
  normalizeTransactionDrafts(const QVariantList &values) const;
  Q_INVOKABLE bool transactionDraftHasContent(const QVariantMap &tx) const;
  Q_INVOKABLE QVariantMap
  createDraftListState(const QVariantList &drafts, int currentIndex,
                       const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap
  insertDraftAfterCurrent(const QVariantList &drafts, int currentIndex,
                          const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap removeDraftAt(const QVariantList &drafts,
                                        int currentIndex,
                                        const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap setCurrentDraft(const QVariantList &drafts,
                                          int currentIndex,
                                          const QVariantMap &draft,
                                          const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap setCurrentRawDraft(const QVariantList &drafts,
                                             int currentIndex,
                                             const QVariantMap &draft,
                                             const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap
  currentDraftState(const QVariantList &drafts, int currentIndex,
                    const QVariantMap &emptyDraft) const;
  Q_INVOKABLE QVariantMap resolveSelectionState(
      const QVariantList &rows, int currentIndex, const QString &selectedId,
      const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantList orderWithInsertedId(const QVariantList &currentOrder,
                                               const QVariantList &availableIds,
                                               const QString &insertedId,
                                               int insertAfterIndex) const;
  Q_INVOKABLE QVariantMap
  orderedRowsState(const QVariantList &rows, const QVariantList &preferredOrder,
                   const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantMap orderedSelectionState(
      const QVariantList &rows, const QVariantList &preferredOrder,
      int currentIndex, const QString &selectedId,
      const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantMap navigateSelectionState(
      const QVariantList &rows, int currentIndex, const QString &selectedId,
      int delta, int defaultIndex = 0,
      const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantMap deleteReselectionState(
      const QVariantList &rows, const QVariantList &preferredOrder,
      int currentIndex, const QString &removedId,
      const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QString deleteNextSelectionId(
      const QVariantList &rows, const QString &removedId, int defaultIndex = 0,
      const QString &idKey = QStringLiteral("id")) const;
  Q_INVOKABLE QVariantMap
  basicFormState(const QString &name, const QVariantList &aliases,
                 const QVariantList &selectedIds = QVariantList()) const;
  Q_INVOKABLE QVariantMap contractFormState(const QString &name,
                                            const QString &type,
                                            const QVariantList &actorIds,
                                            const QVariantList &propertyIds,
                                            const QVariantList &aliases) const;
  Q_INVOKABLE double amountForTransactionCommit(const QVariant &rawAmount,
                                                const QString &transactionId,
                                                double fallbackAmount) const;

signals:
  void dataRevisionChanged();

private:
  void bindModelSignals();
  void bumpDataRevision();

  FilterState filters_;
  SessionModels models_;
  int dataRevision_ = 0;
};

} // namespace ui
