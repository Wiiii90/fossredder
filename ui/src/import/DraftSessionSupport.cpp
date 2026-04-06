/**
 * @file ui/src/import/DraftSessionSupport.cpp
 * @brief Implements draft-session helpers used by import and draft controller flows.
 */

#include "ui/import/DraftSessionSupport.h"

#include <utility>

#include "core/application/AppStateFacade.h"
#include "core/import/DraftLinking.h"
#include "ui/import/DraftViewMapper.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "ui/support/StringConversions.h"

namespace ui::importing {

const TransactionDraft* currentDraft(StatementDraft* draft)
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

core::domain::AppState matchingStateForDraft(const StatementDraft* draft,
                                             const core::application::AppStateFacade* core)
{
    const auto liveState = core ? core->state() : core::domain::AppState{};
    if (!draft) {
        return liveState;
    }

    return core::importing::withFallbackState(
        draft->hasCatalogState() ? draft->catalogState() : core::domain::AppState{},
        liveState);
}

core::domain::DraftStatement buildFinalizationInput(StatementDraft* draft,
                                                    core::application::AppStateFacade* core)
{
    core::domain::DraftStatement input;
    if (!draft || !core) {
        return input;
    }

    const auto& drafts = draft->transactions()->drafts();
    input.name = ui::strings::toStdString(draft->name());
    input.transactions.reserve(drafts.size());

    const auto appState = matchingStateForDraft(draft, core);
    for (const auto& draftTransaction : drafts) {
        core::domain::DraftTransaction transaction;
        transaction.name = ui::strings::toStdString(draftTransaction.name);
        transaction.bookingDate = ui::strings::toStdString(draftTransaction.bookingDate);
        transaction.amount = draftTransaction.amount;
        transaction.description = ui::strings::toStdString(draftTransaction.description);
        transaction.status = static_cast<core::domain::Transaction::Status>(draftTransaction.status);

        QString actorId = draftTransaction.actorId;
        if (actorId.isEmpty() && (!draftTransaction.actorText.trimmed().isEmpty() || draftTransaction.newActorSelected)) {
            actorId = QString::fromStdString(core::importing::resolveActorId(appState, ui::strings::toStdString(draftTransaction.actorText)));
            if (actorId.isEmpty()) {
                const QString actorName = draftTransaction.actorText.trimmed();
                if (!actorName.isEmpty()) {
                    const std::vector<std::string> aliases{ui::strings::toStdString(actorName)};
                    actorId = QString::fromStdString(core->addActor(ui::strings::toStdString(actorName), std::string{}, std::string{}, aliases));
                }
            }
        }

        QString contractId = draftTransaction.contractId;
        if (contractId.isEmpty() && draftTransaction.newContractSelected) {
            const QString contractType = draftTransaction.type.trimmed();
            if (!contractType.isEmpty()) {
                std::vector<std::string> actorIds;
                if (!actorId.isEmpty()) {
                    actorIds.push_back(ui::strings::toStdString(actorId));
                }
                std::vector<std::string> propertyIds = ui::strings::toStdList(draftTransaction.propertyIds);
                std::vector<std::string> aliases = core::importing::referenceAliasesFromMetadata(ui::strings::toStdString(draftTransaction.metadata));
                contractId = QString::fromStdString(core->addContract(std::string{}, ui::strings::toStdString(contractType), std::string{}, actorIds, propertyIds, aliases));
            }
        }

        transaction.actorId = ui::strings::toStdString(actorId);
        transaction.contractId = ui::strings::toStdString(contractId);
        transaction.allocatable = draftTransaction.allocatableManualOverride
            ? draftTransaction.allocatable
            : (core::importing::contractIsFullyAllocatable(appState, ui::strings::toStdString(contractId)) || draftTransaction.allocatable);
        transaction.propertyIds = ui::strings::toStdList(draftTransaction.propertyIds);
        transaction.type = ui::strings::toStdString(draftTransaction.type);
        input.transactions.push_back(std::move(transaction));
    }

    return input;
}

void syncCurrentTransactionDraft(StatementDraft* draft,
                                 core::application::AppStateFacade* core)
{
    const auto* current = currentDraft(draft);
    if (!draft || !current) {
        return;
    }

    const auto appState = matchingStateForDraft(draft, core);
    const auto derived = core::importing::buildDraftDerivedState(appState, toCoreSelection(*current));
    const auto index = draft->currentIndex();
    bool changed = false;

    if (current->actorId.isEmpty()) {
        if (derived.actorCurrentIndex > 0 && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
            const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
            if (!actorRow.synthetic && !actorRow.id.empty()) {
                const QString actorId = QString::fromStdString(actorRow.id);
                const QString actorText = choiceDisplayText(actorRow);
                changed = changed || current->actorId != actorId || current->actorText != actorText || current->newActorSelected;
                draft->transactions()->setActorId(index, actorId);
                draft->transactions()->setActorText(index, actorText);
                draft->transactions()->setNewActorSelected(index, false);
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
                changed = changed || current->contractId != contractId || current->newContractSelected;
                draft->transactions()->setContractId(index, contractId);
                if (!contractRow.type.empty()) {
                    const QString type = QString::fromStdString(contractRow.type);
                    changed = changed || current->type != type;
                    draft->transactions()->setType(index, type);
                }
                if (!contractRow.actorIds.empty()) {
                    const QString actorId = QString::fromStdString(contractRow.actorIds.front());
                    changed = changed || current->actorId != actorId || current->newActorSelected;
                    draft->transactions()->setActorId(index, actorId);
                    if (const auto* actorRow = findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                        const QString actorText = choiceDisplayText(*actorRow);
                        changed = changed || current->actorText != actorText;
                        draft->transactions()->setActorText(index, actorText);
                    }
                    draft->transactions()->setNewActorSelected(index, false);
                }
                if (!contractRow.propertyIds.empty()) {
                    const auto propertyIds = toQStringList(contractRow.propertyIds);
                    changed = changed || current->propertyIds != propertyIds;
                    draft->transactions()->setProperties(index, propertyIds);
                }
                draft->transactions()->setNewContractSelected(index, false);
            }
        } else if (!derived.contractSeedText.empty()) {
            const QString type = QString::fromStdString(derived.contractSeedText);
            changed = changed || current->type != type;
            draft->transactions()->setType(index, type);
        } else if (!current->newContractSelected) {
            changed = changed || !current->type.isEmpty();
            draft->transactions()->setType(index, QString());
        }
    }

    if (current->propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
        const auto propertyIds = toQStringList(derived.autoPropertyIds);
        changed = changed || current->propertyIds != propertyIds;
        draft->transactions()->setProperties(index, propertyIds);
    }

    if (!current->allocatableManualOverride) {
        changed = changed || current->allocatable != derived.effectiveAllocatable;
        draft->transactions()->setAllocatable(index, derived.effectiveAllocatable);
    }

    if (changed) {
        draft->refresh();
    }
}

} // namespace ui::importing
