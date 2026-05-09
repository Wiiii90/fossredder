/**
 * @file ui/src/controllers/DraftController.cpp
 * @brief Implements the UI controller that finalizes imported statement drafts.
 */

#include "ui/controllers/DraftController.h"

#include <QByteArray>

#include "core/application/workspace/WorkspaceFacade.h"
#include "core/application/import/transaction/AmountParser.h"
#include "core/domain/values/Alias.h"
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

core::domain::WorkspaceState matchingStateForDraft(const ui::StatementDraft* draft,
                                             const core::application::WorkspaceFacade* core,
                                             const std::shared_ptr<core::ports::services::IImportMatcherService>& matcherService)
{
    const auto liveState = core ? core->state() : core::domain::WorkspaceState{};
    if (!draft) {
        return liveState;
    }

    return matcherService->withFallbackState(
        draft->hasCatalogState() ? draft->catalogState() : core::domain::WorkspaceState{},
        liveState);
}

core::domain::StatementDraft buildFinalizationInput(ui::StatementDraft* draft,
                                                    core::application::WorkspaceFacade* core,
                                                    const std::shared_ptr<core::ports::services::IImportMatcherService>& matcherService)
{
    core::domain::StatementDraft input;
    if (!draft || !core) {
        return input;
    }

    const auto& drafts = draft->transactions()->drafts();
    input.id = ui::strings::toStdString(draft->draftId());
    input.name = ui::strings::toStdString(draft->name());
    input.transactions.reserve(drafts.size());

    const auto appState = matchingStateForDraft(draft, core, matcherService);
    for (const auto& draftTransaction : drafts) {
        core::domain::TransactionDraft transaction;
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
                    const std::vector<core::domain::Alias> aliases{makeAlias(actorName)};
                    actorId = QString::fromStdString(core->addActor(ui::strings::toStdString(actorName), aliases));
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
                contractId = QString::fromStdString(core->addContract(std::string{}, ui::strings::toStdString(contractType), actorIds, propertyIds, aliases));
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
                                     core::application::WorkspaceFacade* core,
                                     const std::shared_ptr<core::ports::services::IImportMatcherService>& matcherService)
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

DraftController::DraftController(core::application::WorkspaceFacade* core,
                                 std::shared_ptr<core::ports::services::IImportMatcherService> matcherService,
                                 QObject* parent)
    : QObject(parent), core_(core), matcherService_(std::move(matcherService))
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!draft) return {};

    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto input = buildFinalizationInput(draft, core_, matcherService_);
            if (input.transactions.empty()) return QString{};
            return QString::fromStdString(core_->finalizeStatementDraft(input));
        });
}

void DraftController::persistStatementDraft(StatementDraft* draft)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
        if (!draft) return;
        auto input = buildFinalizationInput(draft, core_, matcherService_);
        if (input.transactions.empty()) return;
        core_->saveStatementDraft(input);
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
            if (!current) return QVariantMap{};
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
