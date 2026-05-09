/**
 * @file ui/src/state/StateFacade.cpp
 * @brief Implements the UI session facade that combines session data and selection state.
 */

#include "ui/state/StateFacade.h"

#include "ui/state/StateFacadeProjection.h"

namespace ui {

StateFacade::StateFacade(QObject* parent)
    : QObject(parent)
    , session_(std::make_unique<SessionStore>(this))
    , selection_(std::make_unique<SessionSelection>(session_->models(), this))
{
    QObject::connect(selection_.get(), &SessionSelection::selectedActorIdChanged, this, &StateFacade::selectedActorIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedPropertyIdChanged, this, &StateFacade::selectedPropertyIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedContractIdChanged, this, &StateFacade::selectedContractIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedStatementIdChanged, this, &StateFacade::selectedStatementIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedTransactionIdChanged, this, &StateFacade::selectedTransactionIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnalysisIdChanged, this, &StateFacade::selectedAnalysisIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnnualIdChanged, this, &StateFacade::selectedAnnualIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::lastAnalysisResultChanged, this, &StateFacade::lastAnalysisResultChanged);
}

SessionStore* StateFacade::session() noexcept { return session_.get(); }
SessionSelection* StateFacade::selection() noexcept { return selection_.get(); }

void StateFacade::loadFromState(const WorkspaceState& state)
{
    session_->loadFromState(state);
    selection_->loadFromState();
}

ActorList* StateFacade::actors() noexcept { return &session_->models().actors(); }
PropertyList* StateFacade::properties() noexcept { return &session_->models().properties(); }
ContractList* StateFacade::contracts() noexcept { return &session_->models().contracts(); }
StatementList* StateFacade::statements() noexcept { return &session_->models().statements(); }
TransactionList* StateFacade::transactions() noexcept { return &session_->models().transactions(); }
AnalysisList* StateFacade::analyses() noexcept { return &session_->models().analyses(); }
AnnualList* StateFacade::annuals() noexcept { return &session_->models().annuals(); }

QString StateFacade::selectedActorId() const { return selection_->selectedActorId(); }
QString StateFacade::selectedPropertyId() const { return selection_->selectedPropertyId(); }
QString StateFacade::selectedContractId() const { return selection_->selectedContractId(); }
QString StateFacade::selectedStatementId() const { return selection_->selectedStatementId(); }
QString StateFacade::selectedTransactionId() const { return selection_->selectedTransactionId(); }
QString StateFacade::selectedAnalysisId() const { return selection_->selectedAnalysisId(); }
QString StateFacade::selectedAnnualId() const { return selection_->selectedAnnualId(); }

void StateFacade::setSelectedActorId(const QString& id)
{
    selection_->setSelectedActorId(id);
}

void StateFacade::setSelectedPropertyId(const QString& id)
{
    selection_->setSelectedPropertyId(id);
}

void StateFacade::setSelectedContractId(const QString& id)
{
    selection_->setSelectedContractId(id);
}

void StateFacade::setSelectedStatementId(const QString& id)
{
    selection_->setSelectedStatementId(id);
}

void StateFacade::setSelectedTransactionId(const QString& id)
{
    selection_->setSelectedTransactionId(id);
}

void StateFacade::setSelectedAnalysisId(const QString& id)
{
    selection_->setSelectedAnalysisId(id);
}

void StateFacade::setSelectedAnnualId(const QString& id)
{
    selection_->setSelectedAnnualId(id);
}

ActorSelection* StateFacade::selectedActor() { return selection_->selectedActor(); }
PropertySelection* StateFacade::selectedProperty() { return selection_->selectedProperty(); }
ContractSelection* StateFacade::selectedContract() { return selection_->selectedContract(); }
StatementSelection* StateFacade::selectedStatement() { return selection_->selectedStatement(); }
TransactionSelection* StateFacade::selectedTransaction() { return selection_->selectedTransaction(); }
AnalysisSelection* StateFacade::selectedAnalysis() { return selection_->selectedAnalysis(); }
AnnualSelection* StateFacade::selectedAnnual() { return selection_->selectedAnnual(); }

QVariantList StateFacade::statementTransactionIds(const QString& statementId) const
{
    return buildStatementTransactionIds(*session_, statementId);
}

QVariantList StateFacade::contractRows() const
{
    return buildContractRows(*session_);
}

QVariantList StateFacade::actorRows() const
{
    return buildActorRows(*session_);
}

QVariantList StateFacade::propertyRows() const
{
    return buildPropertyRows(*session_);
}

QVariantList StateFacade::analysisRows() const
{
    return buildAnalysisRows(*session_);
}

QVariantList StateFacade::annualRows() const
{
    return buildAnnualRows(*session_);
}

QVariantList StateFacade::statementRows() const
{
    return buildStatementRows(*session_);
}

QVariantList StateFacade::statementTransactionRows(const QString& statementId) const
{
    return buildStatementTransactionRows(*session_, statementId);
}

QVariantList StateFacade::normalizeStrings(const QVariantList& values) const
{
    return ui::normalizeStrings(values);
}

QVariantList StateFacade::addUniqueTrimmed(const QVariantList& values, const QString& value) const
{
    return ui::addUniqueTrimmed(values, value);
}

QVariantList StateFacade::removeAt(const QVariantList& values, int index) const
{
    return ui::removeAt(values, index);
}

QVariantList StateFacade::removeString(const QVariantList& values, const QString& value) const
{
    return ui::removeString(values, value);
}

QVariantList StateFacade::insertAt(const QVariantList& values, int index, const QVariant& value) const
{
    return ui::insertAt(values, index, value);
}

QVariantList StateFacade::pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds) const
{
    return ui::pruneAndAppendMissing(preferredIds, availableIds);
}

int StateFacade::indexOfId(const QVariantList& rows, const QString& id) const
{
    return ui::indexOfId(rows, id);
}

int StateFacade::indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value) const
{
    return ui::indexOfKeyValue(rows, key, value);
}

int StateFacade::indexOfString(const QVariantList& values, const QString& value) const
{
    return ui::indexOfString(values, value);
}

int StateFacade::normalizedIndex(int index, int count) const
{
    return ui::normalizedIndex(index, count);
}

int StateFacade::wrappedIndex(int index, int count) const
{
    return ui::wrappedIndex(index, count);
}

QString StateFacade::wrappedIdAt(const QVariantList& rows, int index) const
{
    return ui::wrappedIdAt(rows, index);
}

QString StateFacade::navigatedId(const QVariantList& rows,
                                 const QString& currentId,
                                 int delta,
                                 int fallbackIndex) const
{
    return ui::navigatedId(rows, currentId, delta, fallbackIndex);
}

QVariantList StateFacade::displayRowsWithEmpty(const QVariantList& rows,
                                               const QString& emptyDisplay,
                                               const QString& displayKey) const
{
    return ui::displayRowsWithEmpty(rows, emptyDisplay, displayKey);
}

QVariantList StateFacade::rowIds(const QVariantList& rows, const QString& idKey) const
{
    return ui::rowIds(rows, idKey);
}

QVariantList StateFacade::orderedRowsByIds(const QVariantList& rows,
                                           const QVariantList& orderIds,
                                           const QString& idKey) const
{
    return ui::orderedRowsByIds(rows, orderIds, idKey);
}

QVariantMap StateFacade::mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value) const
{
    return ui::mapWithKeyValue(base, key, value);
}

QVariantMap StateFacade::emptyTransactionDraft() const
{
    return ui::emptyTransactionDraft();
}

QVariantMap StateFacade::normalizeTransactionDraft(const QVariantMap& tx) const
{
    return ui::normalizeTransactionDraft(tx);
}

QVariantList StateFacade::normalizeTransactionDrafts(const QVariantList& values) const
{
    return ui::normalizeTransactionDrafts(values);
}

bool StateFacade::transactionDraftHasContent(const QVariantMap& tx) const
{
    return ui::transactionDraftHasContent(tx);
}

QVariantMap StateFacade::createDraftListState(const QVariantList& drafts,
                                              int currentIndex,
                                              const QVariantMap& emptyDraft) const
{
    return ui::createDraftListState(drafts, currentIndex, emptyDraft);
}

QVariantMap StateFacade::insertDraftAfterCurrent(const QVariantList& drafts,
                                                 int currentIndex,
                                                 const QVariantMap& emptyDraft) const
{
    return ui::insertDraftAfterCurrent(drafts, currentIndex, emptyDraft);
}

QVariantMap StateFacade::removeDraftAt(const QVariantList& drafts,
                                       int currentIndex,
                                       const QVariantMap& emptyDraft) const
{
    return ui::removeDraftAt(drafts, currentIndex, emptyDraft);
}

QVariantMap StateFacade::setCurrentDraft(const QVariantList& drafts,
                                         int currentIndex,
                                         const QVariantMap& draft,
                                         const QVariantMap& emptyDraft) const
{
    return ui::setCurrentDraft(drafts, currentIndex, draft, emptyDraft);
}

QVariantMap StateFacade::currentDraftState(const QVariantList& drafts,
                                           int currentIndex,
                                           const QVariantMap& emptyDraft) const
{
    return ui::currentDraftState(drafts, currentIndex, emptyDraft);
}

QVariantMap StateFacade::resolveSelectionState(const QVariantList& rows,
                                               int currentIndex,
                                               const QString& selectedId,
                                               const QString& idKey) const
{
    return ui::resolveSelectionState(rows, currentIndex, selectedId, idKey);
}

QVariantList StateFacade::orderWithInsertedId(const QVariantList& currentOrder,
                                              const QVariantList& availableIds,
                                              const QString& insertedId,
                                              int insertAfterIndex) const
{
    return ui::orderWithInsertedId(currentOrder, availableIds, insertedId, insertAfterIndex);
}

QVariantMap StateFacade::orderedRowsState(const QVariantList& rows,
                                          const QVariantList& preferredOrder,
                                          const QString& idKey) const
{
    return ui::orderedRowsState(rows, preferredOrder, idKey);
}

QVariantMap StateFacade::orderedSelectionState(const QVariantList& rows,
                                               const QVariantList& preferredOrder,
                                               int currentIndex,
                                               const QString& selectedId,
                                               const QString& idKey) const
{
    return ui::orderedSelectionState(rows, preferredOrder, currentIndex, selectedId, idKey);
}

QVariantMap StateFacade::navigateSelectionState(const QVariantList& rows,
                                                int currentIndex,
                                                const QString& selectedId,
                                                int delta,
                                                int fallbackIndex,
                                                const QString& idKey) const
{
    return ui::navigateSelectionState(rows, currentIndex, selectedId, delta, fallbackIndex, idKey);
}

QVariantMap StateFacade::deleteReselectionState(const QVariantList& rows,
                                                const QVariantList& preferredOrder,
                                                int currentIndex,
                                                const QString& removedId,
                                                const QString& idKey) const
{
    return ui::deleteReselectionState(rows, preferredOrder, currentIndex, removedId, idKey);
}

QString StateFacade::deleteNextSelectionId(const QVariantList& rows,
                                           const QString& removedId,
                                           int fallbackIndex,
                                           const QString& idKey) const
{
    return ui::deleteNextSelectionId(rows, removedId, fallbackIndex, idKey);
}

QVariantMap StateFacade::basicFormState(const QString& name,
                                        const QVariantList& aliases,
                                        const QVariantList& selectedIds) const
{
    return ui::basicFormState(name, aliases, selectedIds);
}

QVariantMap StateFacade::contractFormState(const QString& name,
                                           const QString& type,
                                           const QVariantList& actorIds,
                                           const QVariantList& propertyIds,
                                           const QVariantList& aliases) const
{
    return ui::contractFormState(name, type, actorIds, propertyIds, aliases);
}

TransactionFilter* StateFacade::statementTransactions(const QString& statementId)
{
    return session_->statementTransactions(statementId);
}

TransactionFilter* StateFacade::propertyTransactions(const QString& propertyId)
{
    return session_->propertyTransactions(propertyId);
}

void StateFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    session_->applyDeletionImpact(impact);
}

void StateFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_->setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}
