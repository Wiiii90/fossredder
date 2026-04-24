/**
 * @file ui/include/ui/state/StateFacade.h
 * @brief Declares the aggregated session and selection facade exposed to QML.
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <qqmlintegration.h>

#include <memory>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"
#include "ui/models/TransactionFilter.h"
#include "ui/state/SessionSelection.h"
#include "ui/state/SessionStore.h"

namespace ui {

/**
 * @brief Aggregates session data and current selection state for the UI.
 */
class StateFacade : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StateFacade)
    QML_UNCREATABLE("StateFacade is provided by the application context")
    Q_PROPERTY(SessionStore* session READ session CONSTANT)
    Q_PROPERTY(SessionSelection* selection READ selection CONSTANT)
    Q_PROPERTY(ActorList* actors READ actors CONSTANT)
    Q_PROPERTY(PropertyList* properties READ properties CONSTANT)
    Q_PROPERTY(ContractList* contracts READ contracts CONSTANT)
    Q_PROPERTY(StatementList* statements READ statements CONSTANT)
    Q_PROPERTY(TransactionList* transactions READ transactions CONSTANT)
    Q_PROPERTY(AnalysisList* analyses READ analyses CONSTANT)
    Q_PROPERTY(AnnualList* annuals READ annuals CONSTANT)

    Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE setSelectedActorId NOTIFY selectedActorIdChanged)
    Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
    Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE setSelectedContractId NOTIFY selectedContractIdChanged)
    Q_PROPERTY(QString selectedStatementId READ selectedStatementId WRITE setSelectedStatementId NOTIFY selectedStatementIdChanged)
    Q_PROPERTY(QString selectedTransactionId READ selectedTransactionId WRITE setSelectedTransactionId NOTIFY selectedTransactionIdChanged)
    Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId WRITE setSelectedAnalysisId NOTIFY selectedAnalysisIdChanged)
    Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId WRITE setSelectedAnnualId NOTIFY selectedAnnualIdChanged)

    Q_PROPERTY(ActorSelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(PropertySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(ContractSelection* selectedContract READ selectedContract CONSTANT)
    Q_PROPERTY(StatementSelection* selectedStatement READ selectedStatement CONSTANT)
    Q_PROPERTY(TransactionSelection* selectedTransaction READ selectedTransaction CONSTANT)
    Q_PROPERTY(AnalysisSelection* selectedAnalysis READ selectedAnalysis CONSTANT)
    Q_PROPERTY(AnnualSelection* selectedAnnual READ selectedAnnual CONSTANT)

    Q_PROPERTY(QVariant lastAnalysisResult READ lastAnalysisResult WRITE setLastAnalysisResult NOTIFY lastAnalysisResultChanged)

public:
    /** @brief Creates the facade and its owned session state objects. */
    explicit StateFacade(QObject* parent = nullptr);

    /** @brief Returns the session store that owns UI model collections and metrics. */
    SessionStore* session() noexcept;
    /** @brief Returns the selection state synchronized with the current session models. */
    SessionSelection* selection() noexcept;

    ActorList* actors() noexcept;
    PropertyList* properties() noexcept;
    ContractList* contracts() noexcept;
    StatementList* statements() noexcept;
    TransactionList* transactions() noexcept;
    AnalysisList* analyses() noexcept;
    AnnualList* annuals() noexcept;

    /** @brief Loads the UI session from the supplied application state snapshot. */
    void loadFromState(const core::domain::AppState& state);

    QString selectedActorId() const;
    QString selectedPropertyId() const;
    QString selectedContractId() const;
    QString selectedStatementId() const;
    QString selectedTransactionId() const;
    QString selectedAnalysisId() const;
    QString selectedAnnualId() const;

    void setSelectedActorId(const QString& id);
    void setSelectedPropertyId(const QString& id);
    void setSelectedContractId(const QString& id);
    void setSelectedStatementId(const QString& id);
    void setSelectedTransactionId(const QString& id);
    void setSelectedAnalysisId(const QString& id);
    void setSelectedAnnualId(const QString& id);

    ActorSelection* selectedActor();
    PropertySelection* selectedProperty();
    ContractSelection* selectedContract();
    StatementSelection* selectedStatement();
    TransactionSelection* selectedTransaction();
    AnalysisSelection* selectedAnalysis();
    AnnualSelection* selectedAnnual();

    /** @brief Returns transaction ids that belong to the given statement. */
    Q_INVOKABLE QVariantList statementTransactionIds(const QString& statementId) const;
    Q_INVOKABLE QVariantList actorRows() const;
    Q_INVOKABLE QVariantList propertyRows() const;
    Q_INVOKABLE QVariantList contractRows() const;
    Q_INVOKABLE QVariantList analysisRows() const;
    Q_INVOKABLE QVariantList statementRows() const;
    Q_INVOKABLE QVariantList statementTransactionRows(const QString& statementId) const;
    Q_INVOKABLE QVariantList normalizeStrings(const QVariantList& values) const;
    Q_INVOKABLE QVariantList addUniqueTrimmed(const QVariantList& values, const QString& value) const;
    Q_INVOKABLE QVariantList removeAt(const QVariantList& values, int index) const;
    Q_INVOKABLE QVariantList removeString(const QVariantList& values, const QString& value) const;
    Q_INVOKABLE QVariantList insertAt(const QVariantList& values, int index, const QVariant& value) const;
    Q_INVOKABLE QVariantList pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds) const;
    Q_INVOKABLE int indexOfId(const QVariantList& rows, const QString& id) const;
    Q_INVOKABLE int indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value) const;
    Q_INVOKABLE int indexOfString(const QVariantList& values, const QString& value) const;
    Q_INVOKABLE int normalizedIndex(int index, int count) const;
    Q_INVOKABLE int wrappedIndex(int index, int count) const;
    Q_INVOKABLE QString wrappedIdAt(const QVariantList& rows, int index) const;
    Q_INVOKABLE QString navigatedId(const QVariantList& rows,
                                    const QString& currentId,
                                    int delta,
                                    int fallbackIndex = 0) const;
    Q_INVOKABLE QVariantList displayRowsWithEmpty(const QVariantList& rows,
                                                  const QString& emptyDisplay,
                                                  const QString& displayKey = QStringLiteral("display")) const;
    Q_INVOKABLE QVariantList rowIds(const QVariantList& rows, const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantList orderedRowsByIds(const QVariantList& rows,
                                              const QVariantList& orderIds,
                                              const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantMap mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value) const;
    Q_INVOKABLE QVariantMap emptyTransactionDraft() const;
    Q_INVOKABLE QVariantMap normalizeTransactionDraft(const QVariantMap& tx) const;
    Q_INVOKABLE QVariantList normalizeTransactionDrafts(const QVariantList& values) const;
    Q_INVOKABLE bool transactionDraftHasContent(const QVariantMap& tx) const;
    Q_INVOKABLE QVariantMap createDraftListState(const QVariantList& drafts,
                                                 int currentIndex,
                                                 const QVariantMap& emptyDraft) const;
    Q_INVOKABLE QVariantMap insertDraftAfterCurrent(const QVariantList& drafts,
                                                    int currentIndex,
                                                    const QVariantMap& emptyDraft) const;
    Q_INVOKABLE QVariantMap removeDraftAt(const QVariantList& drafts,
                                          int currentIndex,
                                          const QVariantMap& emptyDraft) const;
    Q_INVOKABLE QVariantMap setCurrentDraft(const QVariantList& drafts,
                                            int currentIndex,
                                            const QVariantMap& draft,
                                            const QVariantMap& emptyDraft) const;
    Q_INVOKABLE QVariantMap currentDraftState(const QVariantList& drafts,
                                              int currentIndex,
                                              const QVariantMap& emptyDraft) const;
    Q_INVOKABLE QVariantMap resolveSelectionState(const QVariantList& rows,
                                                  int currentIndex,
                                                  const QString& selectedId,
                                                  const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantList orderWithInsertedId(const QVariantList& currentOrder,
                                                 const QVariantList& availableIds,
                                                 const QString& insertedId,
                                                 int insertAfterIndex) const;
    Q_INVOKABLE QVariantMap orderedRowsState(const QVariantList& rows,
                                             const QVariantList& preferredOrder,
                                             const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantMap orderedSelectionState(const QVariantList& rows,
                                                  const QVariantList& preferredOrder,
                                                  int currentIndex,
                                                  const QString& selectedId,
                                                  const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantMap navigateSelectionState(const QVariantList& rows,
                                                   int currentIndex,
                                                   const QString& selectedId,
                                                   int delta,
                                                   int fallbackIndex = 0,
                                                   const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantMap deleteReselectionState(const QVariantList& rows,
                                                   const QVariantList& preferredOrder,
                                                   int currentIndex,
                                                   const QString& removedId,
                                                   const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QString deleteNextSelectionId(const QVariantList& rows,
                                              const QString& removedId,
                                              int fallbackIndex = 0,
                                              const QString& idKey = QStringLiteral("id")) const;
    Q_INVOKABLE QVariantMap basicFormState(const QString& name,
                                           const QVariantList& aliases,
                                           const QVariantList& selectedIds = QVariantList()) const;
    Q_INVOKABLE QVariantMap contractFormState(const QString& name,
                                              const QString& type,
                                              const QVariantList& actorIds,
                                              const QVariantList& propertyIds,
                                              const QVariantList& aliases) const;
    /** @brief Returns a live filter over transactions that belong to the given statement. */
    Q_INVOKABLE TransactionFilter* statementTransactions(const QString& statementId);
    /** @brief Returns a live filter over transactions assigned to the given property. */
    Q_INVOKABLE TransactionFilter* propertyTransactions(const QString& propertyId);
    /** @brief Returns the distinct contract types currently associated with a property. */
    Q_INVOKABLE QStringList propertyContractTypes(const QString& propertyId) const;
    /** @brief Returns computed transaction sums for a property and optional contract type. */
    Q_INVOKABLE QVariantMap propertyTransactionSums(const QString& propertyId, const QString& contractType = QString()) const;
    /** @brief Resolves a property id to its display name. */
    Q_INVOKABLE QString propertyName(const QString& id) const;

    /** @brief Applies deletion side effects from the domain layer to UI state. */
    Q_INVOKABLE void applyDeletionImpact(const core::domain::DeletionImpact& impact);
    /** @brief Updates property assignments for a transaction without a full reload. */
    Q_INVOKABLE void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

    QVariant lastAnalysisResult() const { return selection_->lastAnalysisResult(); }
    void setLastAnalysisResult(const QVariant& value) { selection_->setLastAnalysisResult(value); }

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();
    void selectedStatementIdChanged();
    void selectedTransactionIdChanged();
    void selectedAnalysisIdChanged();
    void selectedAnnualIdChanged();
    void transactionSumsUpdated(const QString& propertyId);
    void lastAnalysisResultChanged();

private:
    std::unique_ptr<SessionStore> session_;
    std::unique_ptr<SessionSelection> selection_;
};

}
