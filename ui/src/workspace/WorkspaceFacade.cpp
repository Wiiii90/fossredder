/**
 * @file ui/src/workspace/WorkspaceFacade.cpp
 * @brief Implements the UI workspace facade that combines workspace data and selection state.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <exception>

#include "core/application/workspace/WorkspaceFacade.h"
#include "core/domain/values/Alias.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/state/mutation/SessionMutationState.h"
#include "ui/adapters/core/WorkspaceRowProjector.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {

namespace {

std::vector<core::ports::workspace::AliasSnapshot> aliasSnapshots(const QStringList& aliases)
{
    std::vector<core::ports::workspace::AliasSnapshot> out;
    out.reserve(aliases.size());
    for (const auto& alias : aliases) {
        const auto text = strings::toStdString(alias);
        out.push_back({text, {}, text, 0, {}, {}, {}});
    }
    return out;
}

std::vector<std::string> stdStrings(const QStringList& values)
{
    return strings::toStdList(values);
}

core::ports::workspace::ActorCommand makeActorCommand(const QString& id,
                                                      const QString& name,
                                                      const QStringList& aliases)
{
    core::ports::workspace::ActorCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.aliases = aliasSnapshots(aliases);
    return command;
}

core::ports::workspace::PropertyCommand makePropertyCommand(const QString& id,
                                                            const QString& name,
                                                            const QStringList& aliases)
{
    core::ports::workspace::PropertyCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.aliases = aliasSnapshots(aliases);
    return command;
}

core::ports::workspace::ContractCommand makeContractCommand(const QString& id,
                                                            const QString& name,
                                                            const QString& type,
                                                            const QStringList& actorIds,
                                                            const QStringList& propertyIds,
                                                            const QStringList& aliases)
{
    core::ports::workspace::ContractCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.type = strings::toStdString(type);
    command.actorIds = stdStrings(actorIds);
    command.propertyIds = stdStrings(propertyIds);
    command.aliases = aliasSnapshots(aliases);
    return command;
}

core::ports::workspace::TransactionCommand makeTransactionCommand(const QString& id,
                                                                  const QString& name,
                                                                  const QString& bookingDate,
                                                                  double amount,
                                                                  const QString& statementId,
                                                                  int status,
                                                                  const QString& actorId,
                                                                  bool allocatable,
                                                                  const QStringList& propertyIds)
{
    core::ports::workspace::TransactionCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.bookingDate = strings::toStdString(bookingDate);
    command.amount = amount;
    command.statementId = strings::toStdString(statementId);
    command.status = static_cast<core::domain::Transaction::Status>(status);
    command.actorId = strings::toStdString(actorId);
    command.allocatable = allocatable;
    command.propertyIds = stdStrings(propertyIds);
    return command;
}

core::ports::workspace::AnalysisCommand makeAnalysisCommand(const QString& id,
                                                            const QString& name,
                                                            const QString& type,
                                                            const QString& configJson,
                                                            const QString& filterSpec,
                                                            const QString& exportFormat,
                                                            bool includeCalculationAdjustments,
                                                            const QString& exportStateJson,
                                                            const QString& snapshotTransactionsJson)
{
    core::ports::workspace::AnalysisCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.type = strings::toStdString(type);
    command.configJson = strings::toStdString(configJson);
    command.filterSpec = strings::toStdString(filterSpec);
    command.exportFormat = strings::toStdString(exportFormat);
    command.includeCalculationAdjustments = includeCalculationAdjustments;
    command.exportStateJson = strings::toStdString(exportStateJson);
    command.snapshotTransactionsJson = strings::toStdString(snapshotTransactionsJson);
    return command;
}

core::ports::workspace::AnnualCommand makeAnnualCommand(const QString& id,
                                                        const QString& name,
                                                        int year,
                                                        const QStringList& analysisIds)
{
    core::ports::workspace::AnnualCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.year = year;
    command.assignedAnalysisIds = stdStrings(analysisIds);
    return command;
}

} // namespace

WorkspaceFacade::WorkspaceFacade(QObject* parent)
    : QObject(parent)
    , session_(std::make_unique<SessionStore>(this))
    , selection_(std::make_unique<SessionSelection>(session_->models(), this))
{
    QObject::connect(selection_.get(), &SessionSelection::selectedActorIdChanged, this, &WorkspaceFacade::selectedActorIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedPropertyIdChanged, this, &WorkspaceFacade::selectedPropertyIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedContractIdChanged, this, &WorkspaceFacade::selectedContractIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedStatementIdChanged, this, &WorkspaceFacade::selectedStatementIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedTransactionIdChanged, this, &WorkspaceFacade::selectedTransactionIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnalysisIdChanged, this, &WorkspaceFacade::selectedAnalysisIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::selectedAnnualIdChanged, this, &WorkspaceFacade::selectedAnnualIdChanged);
    QObject::connect(selection_.get(), &SessionSelection::lastAnalysisResultChanged, this, &WorkspaceFacade::lastAnalysisResultChanged);
}

WorkspaceFacade::WorkspaceFacade(core::application::WorkspaceFacade* coreFacade, QObject* parent)
    : WorkspaceFacade(parent)
{
    setCoreFacade(coreFacade);
}

SessionStore* WorkspaceFacade::session() noexcept { return session_.get(); }
SessionSelection* WorkspaceFacade::selection() noexcept { return selection_.get(); }

void WorkspaceFacade::setCoreFacade(core::application::WorkspaceFacade* coreFacade) noexcept
{
    coreFacade_ = coreFacade;
    if (!coreFacade_) return;
    loadFromState(coreFacade_->catalogState());
    coreFacade_->setSnapshotChangedCallback([this](const core::ports::workspace::WorkspaceSnapshot& snapshot) {
        core::domain::catalog::WorkspaceCatalog state;
        // Rebuild via the lightweight session path instead of duplicating projection logic.
        // The core snapshot is currently read through the application facade where needed.
        (void)snapshot;
        loadFromState(coreFacade_->catalogState());
    });
}

core::application::WorkspaceFacade* WorkspaceFacade::coreFacade() const noexcept
{
    return coreFacade_;
}

void WorkspaceFacade::loadFromState(const core::domain::catalog::WorkspaceCatalog& state)
{
    session_->loadFromState(state);
    selection_->loadFromState();
}

QString WorkspaceFacade::currentPath() const
{
    return coreFacade_ ? QString::fromStdString(coreFacade_->currentPath()) : QString();
}

void WorkspaceFacade::newFile(const QString& path)
{
    if (!coreFacade_) return;
    try {
        coreFacade_->newFile(strings::toEncodedPath(path));
        emit operationSucceeded(QStringLiteral("newFile"));
    } catch (const std::exception& ex) {
        emit operationFailed(QStringLiteral("newFile"), QString::fromUtf8(ex.what()));
    }
}

void WorkspaceFacade::openFile(const QString& path)
{
    if (!coreFacade_) return;
    try {
        coreFacade_->openFile(strings::toEncodedPath(path));
        emit operationSucceeded(QStringLiteral("openFile"));
    } catch (const std::exception& ex) {
        emit operationFailed(QStringLiteral("openFile"), QString::fromUtf8(ex.what()));
    }
}

void WorkspaceFacade::saveFile()
{
    if (!coreFacade_) return;
    try {
        coreFacade_->saveFile();
        emit operationSucceeded(QStringLiteral("saveFile"));
    } catch (const std::exception& ex) {
        emit operationFailed(QStringLiteral("saveFile"), QString::fromUtf8(ex.what()));
    }
}

void WorkspaceFacade::saveFileAs(const QString& path)
{
    if (!coreFacade_) return;
    try {
        coreFacade_->saveFileAs(strings::toEncodedPath(path));
        emit operationSucceeded(QStringLiteral("saveFileAs"));
    } catch (const std::exception& ex) {
        emit operationFailed(QStringLiteral("saveFileAs"), QString::fromUtf8(ex.what()));
    }
}

ActorList* WorkspaceFacade::actors() noexcept { return &session_->models().actors(); }
PropertyList* WorkspaceFacade::properties() noexcept { return &session_->models().properties(); }
ContractList* WorkspaceFacade::contracts() noexcept { return &session_->models().contracts(); }
StatementList* WorkspaceFacade::statements() noexcept { return &session_->models().statements(); }
TransactionList* WorkspaceFacade::transactions() noexcept { return &session_->models().transactions(); }
AnalysisList* WorkspaceFacade::analyses() noexcept { return &session_->models().analyses(); }
AnnualList* WorkspaceFacade::annuals() noexcept { return &session_->models().annuals(); }

QString WorkspaceFacade::selectedActorId() const { return selection_->selectedActorId(); }
QString WorkspaceFacade::selectedPropertyId() const { return selection_->selectedPropertyId(); }
QString WorkspaceFacade::selectedContractId() const { return selection_->selectedContractId(); }
QString WorkspaceFacade::selectedStatementId() const { return selection_->selectedStatementId(); }
QString WorkspaceFacade::selectedTransactionId() const { return selection_->selectedTransactionId(); }
QString WorkspaceFacade::selectedAnalysisId() const { return selection_->selectedAnalysisId(); }
QString WorkspaceFacade::selectedAnnualId() const { return selection_->selectedAnnualId(); }

void WorkspaceFacade::setSelectedActorId(const QString& id)
{
    selection_->setSelectedActorId(id);
}

void WorkspaceFacade::setSelectedPropertyId(const QString& id)
{
    selection_->setSelectedPropertyId(id);
}

void WorkspaceFacade::setSelectedContractId(const QString& id)
{
    selection_->setSelectedContractId(id);
}

void WorkspaceFacade::setSelectedStatementId(const QString& id)
{
    selection_->setSelectedStatementId(id);
}

void WorkspaceFacade::setSelectedTransactionId(const QString& id)
{
    selection_->setSelectedTransactionId(id);
}

void WorkspaceFacade::setSelectedAnalysisId(const QString& id)
{
    selection_->setSelectedAnalysisId(id);
}

void WorkspaceFacade::setSelectedAnnualId(const QString& id)
{
    selection_->setSelectedAnnualId(id);
}

ActorSelection* WorkspaceFacade::selectedActor() { return selection_->selectedActor(); }
PropertySelection* WorkspaceFacade::selectedProperty() { return selection_->selectedProperty(); }
ContractSelection* WorkspaceFacade::selectedContract() { return selection_->selectedContract(); }
StatementSelection* WorkspaceFacade::selectedStatement() { return selection_->selectedStatement(); }
TransactionSelection* WorkspaceFacade::selectedTransaction() { return selection_->selectedTransaction(); }
AnalysisSelection* WorkspaceFacade::selectedAnalysis() { return selection_->selectedAnalysis(); }
AnnualSelection* WorkspaceFacade::selectedAnnual() { return selection_->selectedAnnual(); }

QVariantList WorkspaceFacade::statementTransactionIds(const QString& statementId) const
{
    return buildStatementTransactionIds(*session_, statementId);
}

QVariantList WorkspaceFacade::contractRows() const
{
    return buildContractRows(*session_);
}

QVariantList WorkspaceFacade::actorRows() const
{
    return buildActorRows(*session_);
}

QVariantList WorkspaceFacade::propertyRows() const
{
    return buildPropertyRows(*session_);
}

QVariantList WorkspaceFacade::analysisRows() const
{
    return buildAnalysisRows(*session_);
}

QVariantList WorkspaceFacade::annualRows() const
{
    return buildAnnualRows(*session_);
}

QVariantList WorkspaceFacade::statementRows() const
{
    return buildStatementRows(*session_);
}

QVariantList WorkspaceFacade::statementTransactionRows(const QString& statementId) const
{
    return buildStatementTransactionRows(*session_, statementId);
}

QVariantList WorkspaceFacade::normalizeStrings(const QVariantList& values) const
{
    return SessionMutationState::normalizeStrings(values);
}

QVariantList WorkspaceFacade::addUniqueTrimmed(const QVariantList& values, const QString& value) const
{
    return SessionMutationState::addUniqueTrimmed(values, value);
}

QVariantList WorkspaceFacade::removeAt(const QVariantList& values, int index) const
{
    return SessionMutationState::removeAt(values, index);
}

QVariantList WorkspaceFacade::removeString(const QVariantList& values, const QString& value) const
{
    return SessionMutationState::removeString(values, value);
}

QVariantList WorkspaceFacade::insertAt(const QVariantList& values, int index, const QVariant& value) const
{
    return SessionMutationState::insertAt(values, index, value);
}

QVariantList WorkspaceFacade::pruneAndAppendMissing(const QVariantList& preferredIds, const QVariantList& availableIds) const
{
    return ui::pruneAndAppendMissing(preferredIds, availableIds);
}

int WorkspaceFacade::indexOfId(const QVariantList& rows, const QString& id) const
{
    return ui::indexOfId(rows, id);
}

int WorkspaceFacade::indexOfKeyValue(const QVariantList& rows, const QString& key, const QVariant& value) const
{
    return ui::indexOfKeyValue(rows, key, value);
}

int WorkspaceFacade::indexOfString(const QVariantList& values, const QString& value) const
{
    return ui::indexOfString(values, value);
}

int WorkspaceFacade::normalizedIndex(int index, int count) const
{
    return ui::normalizedIndex(index, count);
}

int WorkspaceFacade::wrappedIndex(int index, int count) const
{
    return ui::wrappedIndex(index, count);
}

QString WorkspaceFacade::wrappedIdAt(const QVariantList& rows, int index) const
{
    return ui::wrappedIdAt(rows, index);
}

QString WorkspaceFacade::navigatedId(const QVariantList& rows,
                                 const QString& currentId,
                                 int delta,
                                 int fallbackIndex) const
{
    return ui::navigatedId(rows, currentId, delta, fallbackIndex);
}

QVariantList WorkspaceFacade::displayRowsWithEmpty(const QVariantList& rows,
                                               const QString& emptyDisplay,
                                               const QString& displayKey) const
{
    return ui::displayRowsWithEmpty(rows, emptyDisplay, displayKey);
}

QVariantList WorkspaceFacade::rowIds(const QVariantList& rows, const QString& idKey) const
{
    return ui::rowIds(rows, idKey);
}

QVariantList WorkspaceFacade::orderedRowsByIds(const QVariantList& rows,
                                           const QVariantList& orderIds,
                                           const QString& idKey) const
{
    return ui::orderedRowsByIds(rows, orderIds, idKey);
}

QVariantMap WorkspaceFacade::mapWithKeyValue(const QVariantMap& base, const QString& key, const QVariant& value) const
{
    return ui::mapWithKeyValue(base, key, value);
}

QVariantMap WorkspaceFacade::emptyTransactionDraft() const
{
    return SessionMutationState::emptyTransactionDraft();
}

QVariantMap WorkspaceFacade::normalizeTransactionDraft(const QVariantMap& tx) const
{
    return SessionMutationState::normalizeTransactionDraft(tx);
}

QVariantList WorkspaceFacade::normalizeTransactionDrafts(const QVariantList& values) const
{
    return SessionMutationState::normalizeTransactionDrafts(values);
}

bool WorkspaceFacade::transactionDraftHasContent(const QVariantMap& tx) const
{
    return SessionMutationState::transactionDraftHasContent(tx);
}

QVariantMap WorkspaceFacade::createDraftListState(const QVariantList& drafts,
                                              int currentIndex,
                                              const QVariantMap& emptyDraft) const
{
    return SessionMutationState::createDraftListState(drafts, currentIndex, emptyDraft);
}

QVariantMap WorkspaceFacade::insertDraftAfterCurrent(const QVariantList& drafts,
                                                 int currentIndex,
                                                 const QVariantMap& emptyDraft) const
{
    return SessionMutationState::insertDraftAfterCurrent(drafts, currentIndex, emptyDraft);
}

QVariantMap WorkspaceFacade::removeDraftAt(const QVariantList& drafts,
                                       int currentIndex,
                                       const QVariantMap& emptyDraft) const
{
    return SessionMutationState::removeDraftAt(drafts, currentIndex, emptyDraft);
}

QVariantMap WorkspaceFacade::setCurrentDraft(const QVariantList& drafts,
                                         int currentIndex,
                                         const QVariantMap& draft,
                                         const QVariantMap& emptyDraft) const
{
    return SessionMutationState::setCurrentDraft(drafts, currentIndex, draft, emptyDraft);
}

QVariantMap WorkspaceFacade::currentDraftState(const QVariantList& drafts,
                                           int currentIndex,
                                           const QVariantMap& emptyDraft) const
{
    return SessionMutationState::currentDraftState(drafts, currentIndex, emptyDraft);
}

QVariantMap WorkspaceFacade::resolveSelectionState(const QVariantList& rows,
                                               int currentIndex,
                                               const QString& selectedId,
                                               const QString& idKey) const
{
    return ui::resolveSelectionState(rows, currentIndex, selectedId, idKey);
}

QVariantList WorkspaceFacade::orderWithInsertedId(const QVariantList& currentOrder,
                                              const QVariantList& availableIds,
                                              const QString& insertedId,
                                              int insertAfterIndex) const
{
    return ui::orderWithInsertedId(currentOrder, availableIds, insertedId, insertAfterIndex);
}

QVariantMap WorkspaceFacade::orderedRowsState(const QVariantList& rows,
                                          const QVariantList& preferredOrder,
                                          const QString& idKey) const
{
    return ui::orderedRowsState(rows, preferredOrder, idKey);
}

QVariantMap WorkspaceFacade::orderedSelectionState(const QVariantList& rows,
                                               const QVariantList& preferredOrder,
                                               int currentIndex,
                                               const QString& selectedId,
                                               const QString& idKey) const
{
    return ui::orderedSelectionState(rows, preferredOrder, currentIndex, selectedId, idKey);
}

QVariantMap WorkspaceFacade::navigateSelectionState(const QVariantList& rows,
                                                int currentIndex,
                                                const QString& selectedId,
                                                int delta,
                                                int fallbackIndex,
                                                const QString& idKey) const
{
    return ui::navigateSelectionState(rows, currentIndex, selectedId, delta, fallbackIndex, idKey);
}

QVariantMap WorkspaceFacade::deleteReselectionState(const QVariantList& rows,
                                                const QVariantList& preferredOrder,
                                                int currentIndex,
                                                const QString& removedId,
                                                const QString& idKey) const
{
    return ui::deleteReselectionState(rows, preferredOrder, currentIndex, removedId, idKey);
}

QString WorkspaceFacade::deleteNextSelectionId(const QVariantList& rows,
                                           const QString& removedId,
                                           int fallbackIndex,
                                           const QString& idKey) const
{
    return ui::deleteNextSelectionId(rows, removedId, fallbackIndex, idKey);
}

QVariantMap WorkspaceFacade::basicFormState(const QString& name,
                                        const QVariantList& aliases,
                                        const QVariantList& selectedIds) const
{
    return ui::basicFormState(name, aliases, selectedIds);
}

QVariantMap WorkspaceFacade::contractFormState(const QString& name,
                                           const QString& type,
                                           const QVariantList& actorIds,
                                           const QVariantList& propertyIds,
                                           const QVariantList& aliases) const
{
    return ui::contractFormState(name, type, actorIds, propertyIds, aliases);
}

QString WorkspaceFacade::addActor(const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addActor(makeActorCommand({}, name, aliases)));
}

void WorkspaceFacade::updateActor(const QString& id, const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return;
    coreFacade_->updateActor(makeActorCommand(id, name, aliases));
}

QString WorkspaceFacade::saveActor(const QString& id, const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addActor(name, aliases);
    updateActor(id, name, aliases);
    return id;
}

void WorkspaceFacade::deleteActor(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteActor(strings::toStdString(id));
}

QString WorkspaceFacade::addProperty(const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addProperty(makePropertyCommand({}, name, aliases)));
}

void WorkspaceFacade::updateProperty(const QString& id, const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return;
    coreFacade_->updateProperty(makePropertyCommand(id, name, aliases));
}

QString WorkspaceFacade::saveProperty(const QString& id, const QString& name, const QStringList& aliases)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addProperty(name, aliases);
    updateProperty(id, name, aliases);
    return id;
}

void WorkspaceFacade::deleteProperty(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteProperty(strings::toStdString(id));
}

QString WorkspaceFacade::addContract(const QString& name,
                                     const QString& type,
                                     const QStringList& actorIds,
                                     const QStringList& propertyIds,
                                     const QStringList& aliases)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addContract(
        makeContractCommand({}, name, type, actorIds, propertyIds, aliases)));
}

void WorkspaceFacade::updateContract(const QString& id,
                                     const QString& name,
                                     const QString& type,
                                     const QStringList& actorIds,
                                     const QStringList& propertyIds,
                                     const QStringList& aliases)
{
    if (!coreFacade_) return;
    coreFacade_->updateContract(makeContractCommand(id, name, type, actorIds, propertyIds, aliases));
}

QString WorkspaceFacade::saveContract(const QString& id,
                                      const QString& name,
                                      const QString& type,
                                      const QStringList& actorIds,
                                      const QStringList& propertyIds,
                                      const QStringList& aliases)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addContract(name, type, actorIds, propertyIds, aliases);
    updateContract(id, name, type, actorIds, propertyIds, aliases);
    return id;
}

void WorkspaceFacade::deleteContract(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteContract(strings::toStdString(id));
}

QString WorkspaceFacade::addStatement(const QString& name)
{
    if (!coreFacade_) return {};
    core::ports::workspace::StatementCommand command;
    command.name = strings::toStdString(name);
    return QString::fromStdString(coreFacade_->addStatement(command));
}

void WorkspaceFacade::updateStatement(const QString& id, const QString& name)
{
    if (!coreFacade_) return;
    core::ports::workspace::StatementCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    coreFacade_->updateStatement(command);
}

QString WorkspaceFacade::saveStatement(const QString& id, const QString& name)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addStatement(name);
    updateStatement(id, name);
    return id;
}

void WorkspaceFacade::deleteStatement(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteStatement(strings::toStdString(id));
}

QString WorkspaceFacade::addTransaction(const QString& name,
                                        const QString& bookingDate,
                                        double amount,
                                        const QString& statementId,
                                        int status,
                                        const QString& actorId,
                                        bool allocatable,
                                        const QStringList& propertyIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addTransaction(
        makeTransactionCommand({}, name, bookingDate, amount, statementId, status, actorId, allocatable, propertyIds)));
}

void WorkspaceFacade::updateTransaction(const QString& id,
                                        const QString& name,
                                        const QString& bookingDate,
                                        double amount,
                                        const QString& statementId,
                                        int status,
                                        const QString& actorId,
                                        bool allocatable,
                                        const QStringList& propertyIds)
{
    if (!coreFacade_) return;
    coreFacade_->updateTransaction(makeTransactionCommand(id, name, bookingDate, amount, statementId, status, actorId, allocatable, propertyIds));
}

QString WorkspaceFacade::saveTransaction(const QString& id,
                                         const QString& name,
                                         const QString& bookingDate,
                                         double amount,
                                         const QString& statementId,
                                         int status,
                                         const QString& actorId,
                                         bool allocatable,
                                         const QStringList& propertyIds)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addTransaction(name, bookingDate, amount, statementId, status, actorId, allocatable, propertyIds);
    updateTransaction(id, name, bookingDate, amount, statementId, status, actorId, allocatable, propertyIds);
    return id;
}

void WorkspaceFacade::deleteTransaction(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteTransaction(strings::toStdString(id));
}

QString WorkspaceFacade::addAnalysis(const QString& name,
                                     const QString& type,
                                     const QString& configJson,
                                     const QString& filterSpec,
                                     const QString& exportFormat,
                                     bool includeCalculationAdjustments,
                                     const QString& exportStateJson,
                                     const QString& snapshotTransactionsJson)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addAnalysis(
        makeAnalysisCommand({}, name, type, configJson, filterSpec, exportFormat,
                            includeCalculationAdjustments, exportStateJson, snapshotTransactionsJson)));
}

void WorkspaceFacade::updateAnalysis(const QString& id,
                                     const QString& name,
                                     const QString& type,
                                     const QString& configJson,
                                     const QString& filterSpec,
                                     const QString& exportFormat,
                                     bool includeCalculationAdjustments,
                                     const QString& exportStateJson,
                                     const QString& snapshotTransactionsJson)
{
    if (!coreFacade_) return;
    coreFacade_->updateAnalysis(makeAnalysisCommand(id, name, type, configJson, filterSpec, exportFormat,
                                                    includeCalculationAdjustments, exportStateJson, snapshotTransactionsJson));
}

void WorkspaceFacade::deleteAnalysis(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteAnalysis(strings::toStdString(id));
}

QString WorkspaceFacade::addAnnual(const QString& name, int year, const QStringList& analysisIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addAnnual(makeAnnualCommand({}, name, year, analysisIds)));
}

void WorkspaceFacade::updateAnnual(const QString& id, const QString& name, int year, const QStringList& analysisIds)
{
    if (!coreFacade_) return;
    coreFacade_->updateAnnual(makeAnnualCommand(id, name, year, analysisIds));
}

QString WorkspaceFacade::saveAnnual(const QString& id, const QString& name, int year, const QStringList& analysisIds)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addAnnual(name, year, analysisIds);
    updateAnnual(id, name, year, analysisIds);
    return id;
}

void WorkspaceFacade::deleteAnnual(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteAnnual(strings::toStdString(id));
}

TransactionFilter* WorkspaceFacade::statementTransactions(const QString& statementId)
{
    return session_->statementTransactions(statementId);
}

TransactionFilter* WorkspaceFacade::propertyTransactions(const QString& propertyId)
{
    return session_->propertyTransactions(propertyId);
}

void WorkspaceFacade::applyDeletionImpact(const DeletionImpact& impact)
{
    session_->applyDeletionImpact(impact);
}

void WorkspaceFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_->setTransactionPropertyIdsImmediate(txId, propertyIds);
}

}

