/**
 * @file ui/src/workspace/WorkspaceFacade.cpp
 * @brief Implements the UI workspace facade that combines workspace data and selection state.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <exception>

#include "core/application/workspace/WorkspaceFacade.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/adapters/core/EntityPayloadMapper.h"
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
                                                      const QStringList& aliases,
                                                      const QStringList& contractIds)
{
    core::ports::workspace::ActorCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.aliases = aliasSnapshots(aliases);
    command.contractIds = stdStrings(contractIds);
    return command;
}

core::ports::workspace::PropertyCommand makePropertyCommand(const QString& id,
                                                            const QString& name,
                                                            const QStringList& aliases,
                                                            const QStringList& contractIds)
{
    core::ports::workspace::PropertyCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.aliases = aliasSnapshots(aliases);
    command.contractIds = stdStrings(contractIds);
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
                                                                  const QString& valuta,
                                                                  double amount,
                                                                  const QString& statementId,
                                                                  const QString& insertAfterTransactionId,
                                                                  int status,
                                                                  const QString& actorId,
                                                                  const QString& contractId,
                                                                  bool allocatable,
                                                                  const QStringList& propertyIds)
{
    core::ports::workspace::TransactionCommand command;
    command.id = strings::toStdString(id);
    command.name = strings::toStdString(name);
    command.bookingDate = strings::toStdString(bookingDate);
    command.valuta = strings::toStdString(valuta);
    command.amount = amount;
    command.statementId = strings::toStdString(statementId);
    command.insertAfterTransactionId = strings::toStdString(insertAfterTransactionId);
    command.status = static_cast<core::domain::Transaction::Status>(status);
    command.actorId = strings::toStdString(actorId);
    command.contractId = strings::toStdString(contractId);
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
    , session_(std::make_unique<SessionState>(this))
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

SessionState* WorkspaceFacade::session() noexcept { return session_.get(); }
SessionSelection* WorkspaceFacade::selection() noexcept { return selection_.get(); }

void WorkspaceFacade::bumpDataRevision()
{
    ++dataRevision_;
    emit dataRevisionChanged();
}

void WorkspaceFacade::setCoreFacade(core::application::WorkspaceFacade* coreFacade) noexcept
{
    coreFacade_ = coreFacade;
    if (!coreFacade_) return;
    loadFromState(coreFacade_->state());
    coreFacade_->setSnapshotChangedCallback([this](const core::ports::workspace::WorkspaceSnapshot& snapshot) {
        (void)snapshot;
        loadFromState(coreFacade_->state());
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
    bumpDataRevision();
}

void WorkspaceFacade::loadFromState(const core::application::workspace::WorkspaceSessionState& state)
{
    loadFromState(state.catalog);
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

QVariantMap WorkspaceFacade::transaction(const QString& id) const
{
    QVariantMap out;
    if (!session_ || id.isEmpty()) return out;

    const auto& model = session_->models().transactions();
    const int row = model.findRowById(id);
    if (row < 0) return out;
    return model.get(row);
}

QVariantMap WorkspaceFacade::annual(const QString& id) const
{
    QVariantMap out;
    if (!session_ || id.isEmpty()) return out;

    const auto& rows = session_->models().annuals().annuals();
    for (const auto& annual : rows) {
        if (!annual) continue;
        if (QString::fromStdString(annual->id()) != id) continue;

        out = payload::entity::toPayload(*annual);
        out.insert(QStringLiteral("display"),
                   annual->name().empty() ? QString::number(annual->year())
                                          : QString::fromStdString(annual->name()));
        return out;
    }

    return out;
}

QString WorkspaceFacade::addActor(const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addActor(makeActorCommand({}, name, aliases, contractIds)));
}

void WorkspaceFacade::updateActor(const QString& id, const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return;
    coreFacade_->updateActor(makeActorCommand(id, name, aliases, contractIds));
}

QString WorkspaceFacade::saveActor(const QString& id, const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addActor(name, aliases, contractIds);
    updateActor(id, name, aliases, contractIds);
    return id;
}

void WorkspaceFacade::deleteActor(const QString& id)
{
    if (coreFacade_) coreFacade_->deleteActor(strings::toStdString(id));
}

QString WorkspaceFacade::addProperty(const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addProperty(makePropertyCommand({}, name, aliases, contractIds)));
}

void WorkspaceFacade::updateProperty(const QString& id, const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return;
    coreFacade_->updateProperty(makePropertyCommand(id, name, aliases, contractIds));
}

QString WorkspaceFacade::saveProperty(const QString& id, const QString& name, const QStringList& aliases, const QStringList& contractIds)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addProperty(name, aliases, contractIds);
    updateProperty(id, name, aliases, contractIds);
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
                                        const QString& valuta,
                                        double amount,
                                        const QString& statementId,
                                        int status,
                                        const QString& actorId,
                                        const QString& contractId,
                                        bool allocatable,
                                        const QStringList& propertyIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addTransaction(
        makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId, {}, status, actorId, contractId, allocatable, propertyIds)));
}

QString WorkspaceFacade::insertTransactionAfter(const QString& afterTransactionId,
                                                const QString& name,
                                                const QString& bookingDate,
                                                const QString& valuta,
                                                double amount,
                                                const QString& statementId,
                                                int status,
                                                const QString& actorId,
                                                const QString& contractId,
                                                bool allocatable,
                                                const QStringList& propertyIds)
{
    if (!coreFacade_) return {};
    return QString::fromStdString(coreFacade_->addTransaction(
        makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId, afterTransactionId, status, actorId, contractId, allocatable, propertyIds)));
}

void WorkspaceFacade::updateTransaction(const QString& id,
                                        const QString& name,
                                        const QString& bookingDate,
                                        const QString& valuta,
                                        double amount,
                                        const QString& statementId,
                                        int status,
                                        const QString& actorId,
                                        const QString& contractId,
                                        bool allocatable,
                                        const QStringList& propertyIds)
{
    if (!coreFacade_) return;
    coreFacade_->updateTransaction(makeTransactionCommand(id, name, bookingDate, valuta, amount, statementId, {}, status, actorId, contractId, allocatable, propertyIds));
}

QString WorkspaceFacade::saveTransaction(const QString& id,
                                         const QString& name,
                                         const QString& bookingDate,
                                         const QString& valuta,
                                         double amount,
                                         const QString& statementId,
                                         int status,
                                         const QString& actorId,
                                         const QString& contractId,
                                         bool allocatable,
                                         const QStringList& propertyIds)
{
    if (!coreFacade_) return {};
    if (id.isEmpty()) return addTransaction(name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds);
    updateTransaction(id, name, bookingDate, valuta, amount, statementId, status, actorId, contractId, allocatable, propertyIds);
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
    bumpDataRevision();
}

void WorkspaceFacade::setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds)
{
    session_->setTransactionPropertyIdsImmediate(txId, propertyIds);
    bumpDataRevision();
}

}

