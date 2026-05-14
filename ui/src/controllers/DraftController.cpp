/**
 * @file ui/src/controllers/DraftController.cpp
 * @brief Implements the UI controller that finalizes imported statement drafts.
 */

#include "ui/controllers/DraftController.h"

#include <QByteArray>

#include "core/application/import/draft/IImportMatcherService.h"
#include "core/application/import/transaction/AmountParser.h"
#include "core/domain/values/Alias.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/import/DraftViewMapper.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "ui/observability/Origins.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

#include <QVariantList>

namespace {

core::domain::Alias makeAlias(const QString& value)
{
    const std::string stdValue = ui::strings::toStdString(value);
    return core::domain::Alias{stdValue, {}, stdValue, {}, {}};
}

const ui::TransactionDraft* currentDraft(ui::StatementDraft* draft)
{
    if (!draft || !draft->hasCurrent()) {
        return nullptr;
    }

    const auto index = draft->currentIndex();
    const auto& drafts = draft->transactions()->drafts();
    if (index < 0 || static_cast<std::size_t>(index) >= drafts.size()) {
        return nullptr;
    }
    return &drafts[static_cast<std::size_t>(index)];
}

core::domain::catalog::WorkspaceCatalog toWorkspaceState(const core::ports::workspace::WorkspaceSnapshot& snapshot)
{
    core::domain::catalog::WorkspaceCatalog state;

    core::domain::catalog::WorkspaceCatalog::ActorList actors;
    actors.reserve(snapshot.actors.size());
    for (const auto& src : snapshot.actors) {
        auto entity = std::make_shared<core::domain::Actor>();
        entity->setId(src.id);
        entity->rename(src.name);
        std::vector<core::domain::Alias> aliases;
        aliases.reserve(src.aliases.size());
        for (const auto& alias : src.aliases) {
            aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
        }
        entity->setAliases(std::move(aliases));
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        actors.push_back(std::move(entity));
    }
    state.setActors(std::move(actors));

    core::domain::catalog::WorkspaceCatalog::PropertyList properties;
    properties.reserve(snapshot.properties.size());
    for (const auto& src : snapshot.properties) {
        auto entity = std::make_shared<core::domain::Property>();
        entity->setId(src.id);
        entity->rename(src.name);
        std::vector<core::domain::Alias> aliases;
        aliases.reserve(src.aliases.size());
        for (const auto& alias : src.aliases) {
            aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
        }
        entity->setAliases(std::move(aliases));
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        properties.push_back(std::move(entity));
    }
    state.setProperties(std::move(properties));

    core::domain::catalog::WorkspaceCatalog::ContractList contracts;
    contracts.reserve(snapshot.contracts.size());
    for (const auto& src : snapshot.contracts) {
        auto entity = std::make_shared<core::domain::Contract>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setType(src.type);
        entity->setActorIds(src.actorIds);
        entity->setPropertyIds(src.propertyIds);
        std::vector<core::domain::Alias> aliases;
        aliases.reserve(src.aliases.size());
        for (const auto& alias : src.aliases) {
            aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
        }
        entity->setAliases(std::move(aliases));
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        contracts.push_back(std::move(entity));
    }
    state.setContracts(std::move(contracts));

    core::domain::catalog::WorkspaceCatalog::StatementList statements;
    statements.reserve(snapshot.statements.size());
    for (const auto& src : snapshot.statements) {
        auto entity = std::make_shared<core::domain::Statement>();
        entity->setId(src.id);
        entity->rename(src.name);
        entity->setTransactionIds(src.transactionIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        statements.push_back(std::move(entity));
    }
    state.setStatements(std::move(statements));

    core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
    transactions.reserve(snapshot.transactions.size());
    for (const auto& src : snapshot.transactions) {
        auto entity = std::make_shared<core::domain::Transaction>();
        entity->setId(src.id);
        entity->setName(src.name);
        entity->setBookingDate(src.bookingDate);
        entity->setValuta(src.valuta);
        entity->setAmount(src.amount);
        entity->setStatus(src.status);
        entity->setContractId(src.contractId);
        entity->setActorId(src.actorId);
        entity->setStatementId(src.statementId);
        entity->setAllocatable(src.allocatable);
        entity->setPropertyIds(src.propertyIds);
        entity->setCreatedAt(src.createdAt);
        entity->setUpdatedAt(src.updatedAt);
        transactions.push_back(std::move(entity));
    }
    state.setTransactions(std::move(transactions));

    return state;
}

core::ports::workspace::StatementDraftSnapshot toSnapshot(const core::application::importing::draft::StatementDraft& draft)
{
    core::ports::workspace::StatementDraftSnapshot snapshot;
    snapshot.id = draft.id;
    snapshot.name = draft.name;
    snapshot.transactionIds = draft.transactionIds;
    snapshot.createdAt = draft.createdAt;
    snapshot.updatedAt = draft.updatedAt;
    snapshot.transactions.reserve(draft.transactions.size());
    for (const auto& tx : draft.transactions) {
        core::ports::workspace::TransactionDraftSnapshot txSnapshot;
        txSnapshot.id = tx.id;
        txSnapshot.statementDraftId = tx.statementDraftId;
        txSnapshot.name = tx.name;
        txSnapshot.bookingDate = tx.bookingDate;
        txSnapshot.valuta = tx.valuta;
        txSnapshot.amount = tx.amount;
        txSnapshot.actorId = tx.actorId;
        txSnapshot.contractId = tx.contractId;
        txSnapshot.propertyIds = tx.propertyIds;
        txSnapshot.status = static_cast<int>(tx.status);
        txSnapshot.allocatable = tx.allocatable;
        txSnapshot.position = tx.position;
        txSnapshot.metadata = tx.metadata;
        snapshot.transactions.push_back(std::move(txSnapshot));
    }
    return snapshot;
}

core::domain::catalog::WorkspaceCatalog matchingStateForDraft(const ui::StatementDraft* draft,
                                                              const core::ports::workspace::IWorkspaceWriter* core,
                                                              const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService)
{
    const auto reader = dynamic_cast<const core::ports::workspace::IWorkspaceReader*>(core);
    const auto liveState = reader ? toWorkspaceState(reader->workspaceSnapshot()) : core::domain::catalog::WorkspaceCatalog{};
    if (!draft) {
        return liveState;
    }

    return matcherService->withFallbackState(
        draft->hasCatalogState() ? draft->catalogState() : core::domain::catalog::WorkspaceCatalog{},
        liveState);
}

core::application::importing::draft::StatementDraft buildFinalizationInput(ui::StatementDraft* draft,
                                                                            core::ports::workspace::IWorkspaceWriter* core,
                                                                           const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService)
{
    core::application::importing::draft::StatementDraft input;
    if (!draft || !core) {
        return input;
    }

    const auto& drafts = draft->transactions()->drafts();
    input.id = ui::strings::toStdString(draft->draftId());
    input.name = ui::strings::toStdString(draft->name());
    input.transactions.reserve(drafts.size());

    const auto appState = matchingStateForDraft(draft, core, matcherService);
    for (const auto& draftTransaction : drafts) {
        core::application::importing::draft::TransactionDraft transaction;
        transaction.name = ui::strings::toStdString(draftTransaction.name);
        transaction.bookingDate = ui::strings::toStdString(draftTransaction.bookingDate);
        transaction.valuta = ui::strings::toStdString(draftTransaction.valuta);
        transaction.amount = draftTransaction.amount;
        if (!draftTransaction.proofImageData.isEmpty()) {
            const QByteArray imageBytes = QByteArray::fromBase64(draftTransaction.proofImageData.toLatin1());
            if (!imageBytes.isEmpty()) {
                transaction.proofImageData.assign(imageBytes.begin(), imageBytes.end());
            }
        }
        transaction.status = static_cast<core::domain::Transaction::Status>(draftTransaction.status);

        QString actorId = draftTransaction.actorId;
        if (actorId.isEmpty() && (!draftTransaction.actorText.trimmed().isEmpty() || draftTransaction.actorSelected)) {
            actorId = QString::fromStdString(matcherService->resolveActorId(appState, ui::strings::toStdString(draftTransaction.actorText)));
            if (actorId.isEmpty()) {
                const QString actorName = draftTransaction.actorText.trimmed();
                if (!actorName.isEmpty()) {
                    core::ports::workspace::ActorCommand command;
                    command.name = ui::strings::toStdString(actorName);
                    const auto alias = makeAlias(actorName);
                    command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
                    actorId = QString::fromStdString(core->addActor(command));
                }
            }
        }

        QString contractId = draftTransaction.contractId;
        if (contractId.isEmpty() && draftTransaction.contractSelected) {
            const QString contractType = draftTransaction.type.trimmed();
            if (!contractType.isEmpty()) {
                std::vector<std::string> actorIds;
                if (!actorId.isEmpty()) {
                    actorIds.push_back(ui::strings::toStdString(actorId));
                }
                std::vector<std::string> propertyIds = ui::strings::toStdList(draftTransaction.propertyIds);
                std::vector<core::domain::Alias> aliases;
                for (const auto& value : matcherService->referenceAliasesFromMetadata(ui::strings::toStdString(draftTransaction.metadata))) {
                    aliases.push_back(core::domain::Alias{value, {}, value, {}, {}});
                }
                core::ports::workspace::ContractCommand command;
                command.name = {};
                command.type = ui::strings::toStdString(contractType);
                command.actorIds = std::move(actorIds);
                command.propertyIds = std::move(propertyIds);
                for (const auto& alias : aliases) {
                    command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
                }
                contractId = QString::fromStdString(core->addContract(command));
            }
        }

        transaction.actorId = ui::strings::toStdString(actorId);
        transaction.contractId = ui::strings::toStdString(contractId);
        transaction.allocatable = draftTransaction.allocatableSelected
            ? draftTransaction.allocatable
            : (matcherService->contractIsFullyAllocatable(appState, ui::strings::toStdString(contractId)) || draftTransaction.allocatable);
        transaction.propertyIds = ui::strings::toStdList(draftTransaction.propertyIds);
        transaction.type = ui::strings::toStdString(draftTransaction.type);
        input.transactions.push_back(std::move(transaction));
    }

    return input;
}

void syncCurrentTransactionDraftImpl(ui::StatementDraft* draft,
                                     core::ports::workspace::IWorkspaceWriter* core,
                                     const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService)
{
    const auto* current = currentDraft(draft);
    if (!draft || !current) {
        return;
    }

    const auto appState = matchingStateForDraft(draft, core, matcherService);
    const auto derived = matcherService->buildDraftDerivedState(appState, ui::importing::toCoreSelection(*current));
    const auto index = draft->currentIndex();
    bool changed = false;

    if (current->actorId.isEmpty()) {
        if (derived.actorCurrentIndex > 0 && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
            const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
            if (!actorRow.synthetic && !actorRow.id.empty()) {
                const QString actorId = QString::fromStdString(actorRow.id);
                const QString actorText = ui::importing::choiceDisplayText(actorRow);
                changed = changed || current->actorId != actorId || current->actorText != actorText || current->actorSelected;
                draft->transactions()->setActorId(index, actorId);
                draft->transactions()->setActorText(index, actorText);
                draft->transactions()->setActorSelected(index, false);
            }
        } else if (!derived.actorSeedText.empty()) {
            const QString actorText = QString::fromStdString(derived.actorSeedText);
            changed = changed || current->actorText != actorText;
            draft->transactions()->setActorText(index, actorText);
        }
    }

    if (current->contractId.isEmpty()) {
        if (derived.contractCurrentIndex > 0 && static_cast<std::size_t>(derived.contractCurrentIndex) < derived.contractChoices.size()) {
            const auto& contractRow = derived.contractChoices[static_cast<std::size_t>(derived.contractCurrentIndex)];
            if (!contractRow.synthetic && !contractRow.id.empty()) {
                const QString contractId = QString::fromStdString(contractRow.id);
                changed = changed || current->contractId != contractId || current->contractSelected;
                draft->transactions()->setContractId(index, contractId);
                if (!contractRow.type.empty()) {
                    const QString type = QString::fromStdString(contractRow.type);
                    changed = changed || current->type != type;
                    draft->transactions()->setType(index, type);
                }
                if (!contractRow.actorIds.empty()) {
                    const QString actorId = QString::fromStdString(contractRow.actorIds.front());
                    changed = changed || current->actorId != actorId || current->actorSelected;
                    draft->transactions()->setActorId(index, actorId);
                    if (const auto* actorRow = ui::importing::findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                        const QString actorText = ui::importing::choiceDisplayText(*actorRow);
                        changed = changed || current->actorText != actorText;
                        draft->transactions()->setActorText(index, actorText);
                    }
                    draft->transactions()->setActorSelected(index, false);
                }
                if (!contractRow.propertyIds.empty()) {
                    const auto propertyIds = ui::payload::mapper::toQStringList(contractRow.propertyIds);
                    changed = changed || current->propertyIds != propertyIds;
                    draft->transactions()->setProperties(index, propertyIds);
                }
                draft->transactions()->setContractSelected(index, true);
            }
        } else if (!derived.contractSeedText.empty()) {
            const QString type = QString::fromStdString(derived.contractSeedText);
            changed = changed || current->type != type;
            draft->transactions()->setType(index, type);
        } else if (!current->contractSelected) {
            changed = changed || !current->type.isEmpty();
            draft->transactions()->setType(index, QString());
        }
    }

    if (current->propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
        const auto propertyIds = ui::payload::mapper::toQStringList(derived.autoPropertyIds);
        changed = changed || current->propertyIds != propertyIds;
        draft->transactions()->setProperties(index, propertyIds);
    }

    if (!current->allocatableSelected) {
        changed = changed || current->allocatable != derived.effectiveAllocatable;
        draft->transactions()->setAllocatable(index, derived.effectiveAllocatable);
    }

    if (changed) {
        draft->refresh();
    }
}

} // namespace

namespace ui {

DraftController::DraftController(core::ports::workspace::IWorkspaceWriter* core,
                                 std::shared_ptr<core::application::importing::draft::IImportMatcherService> matcherService,
                                 QObject* parent)
    : QObject(parent), core_(core), reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core)), matcherService_(std::move(matcherService))
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!draft) return {};

    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto input = buildFinalizationInput(draft, core_, matcherService_);
            if (input.transactions.empty()) return QString{};
            core::ports::workspace::FinalizeStatementDraftCommand command;
            command.draft = toSnapshot(input);
            return QString::fromStdString(core_->finalizeStatementDraft(command));
        });
}

void DraftController::persistStatementDraft(StatementDraft* draft)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
        if (!draft) return;
        auto input = buildFinalizationInput(draft, core_, matcherService_);
        if (input.transactions.empty()) return;
        core::ports::workspace::StatementDraftCommand command;
        command.draft = toSnapshot(input);
        core_->saveStatementDraft(command);
    });
}

void DraftController::clearPersistedStatementDraft(const QString& draftId)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
        core_->clearStatementDraft(ui::strings::toStdString(draftId));
    });
}

QVariantMap DraftController::currentTransactionViewState(StatementDraft* draft) const
{
    return ui::util::guard::invokeValue<QVariantMap>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto* current = currentDraft(draft);
            if (!current || !matcherService_) return QVariantMap{};
            return ui::importing::toViewState(matcherService_->buildDraftDerivedState(
                matchingStateForDraft(draft, core_, matcherService_),
                ui::importing::toCoreSelection(*current)));
        });
}

QVariantMap DraftController::findChoiceRowByText(const QVariantList& rows, const QString& text) const
{
    for (const auto& item : rows) {
        const QVariantMap row = item.toMap();
        if (!row.isEmpty() && ui::importing::rowMatchesText(row, text)) return row;
    }
    return {};
}

void DraftController::syncCurrentTransactionDraft(StatementDraft* draft)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
            syncCurrentTransactionDraftImpl(draft, core_, matcherService_);
    });
}

void DraftController::selectCurrentActorChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setActorId(draft->currentIndex(), QString());
        draft->transactions()->setActorText(draft->currentIndex(), QString());
        draft->transactions()->setActorSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setActorId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        draft->transactions()->setActorText(draft->currentIndex(), importing::rowDisplayText(row));
        draft->transactions()->setActorSelected(draft->currentIndex(), false);
    }
}

void DraftController::selectCurrentContractChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setContractId(draft->currentIndex(), QString());
        draft->transactions()->setType(draft->currentIndex(), QString());
        draft->transactions()->setContractSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setContractId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        const QString type = row.value(QStringLiteral("type")).toString();
        if (!type.isEmpty()) draft->transactions()->setType(draft->currentIndex(), type);

        const auto actorIds = row.value(QStringLiteral("actorIds")).toStringList();
        if (!actorIds.isEmpty()) draft->transactions()->setActorId(draft->currentIndex(), actorIds.first());

        const auto propertyIds = row.value(QStringLiteral("propertyIds")).toStringList();
        if (!propertyIds.isEmpty()) draft->transactions()->setProperties(draft->currentIndex(), propertyIds);

        draft->transactions()->setContractSelected(draft->currentIndex(), false);
    }
}

void DraftController::setCurrentPropertySelected(StatementDraft* draft, const QString& propertyId, bool selected)
{
    const auto* current = currentDraft(draft);
    if (!draft || !current || propertyId.isEmpty()) return;

    QStringList propertyIds = current->propertyIds;
    const int index = propertyIds.indexOf(propertyId);
    if (selected) {
        if (index < 0) propertyIds.push_back(propertyId);
    } else if (index >= 0) {
        propertyIds.removeAt(index);
    }

    draft->transactions()->setProperties(draft->currentIndex(), propertyIds);
}

void DraftController::updateCurrentAmount(StatementDraft* draft, const QString& text)
{
    if (!draft) return;

    const auto parsed = core::parser::parseAmountString(text.trimmed().toStdString());
    if (!parsed) {
        draft->transactions()->setAmount(draft->currentIndex(), 0.0);
        return;
    }

    draft->transactions()->setAmount(draft->currentIndex(), *parsed);
}

} // namespace ui
