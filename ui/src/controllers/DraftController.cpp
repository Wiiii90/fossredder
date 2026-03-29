/**
 * @file ui/src/controllers/DraftController.cpp
 * @brief Implements the UI controller that finalizes imported statement drafts.
 */

#include "ui/controllers/DraftController.h"

#include "core/application/AppStateFacade.h"
#include "core/import/DraftLinking.h"
#include "core/import/parsing/AmountParser.h"
#include "core/models/DraftStatement.h"
#include "ui/controllers/DraftProjection.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "ui/observability/Origins.h"

#include <QVariantList>

namespace ui {

DraftController::DraftController(core::application::AppStateFacade* core, QObject* parent)
    : QObject(parent), core_(core)
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!draft) return {};

    return controllers::guard::invokeValue<QString>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto& drafts = draft->transactions()->drafts();
            if (drafts.empty()) return QString{};
            const auto appState = matchingStateForDraft(draft, core_);

            DraftStatement input;
            input.name = strings::toStdString(draft->name());
            input.transactions.reserve(drafts.size());

            for (const auto& d : drafts) {
                DraftTransaction transaction;
                transaction.name = strings::toStdString(d.name);
                transaction.bookingDate = strings::toStdString(d.bookingDate);
                transaction.amount = d.amount;
                transaction.description = strings::toStdString(d.description);
                transaction.status = static_cast<Transaction::Status>(d.status);

                QString actorId = d.actorId;
                if (actorId.isEmpty() && (!d.actorText.trimmed().isEmpty() || d.newActorSelected)) {
                    actorId = QString::fromStdString(core::importing::resolveActorId(appState, strings::toStdString(d.actorText)));
                    if (actorId.isEmpty()) {
                        const QString actorName = d.actorText.trimmed();
                        if (!actorName.isEmpty()) {
                            const std::vector<std::string> aliases{strings::toStdString(actorName)};
                            actorId = QString::fromStdString(core_->addActor(strings::toStdString(actorName), std::string{}, std::string{}, aliases));
                        }
                    }
                }

                QString contractId = d.contractId;
                if (contractId.isEmpty() && d.newContractSelected) {
                    const QString contractType = d.type.trimmed();
                    if (!contractType.isEmpty()) {
                        std::vector<std::string> actorIds;
                        if (!actorId.isEmpty()) actorIds.push_back(strings::toStdString(actorId));
                        std::vector<std::string> propertyIds = strings::toStdList(d.propertyIds);
                        std::vector<std::string> aliases = core::importing::referenceAliasesFromMetadata(strings::toStdString(d.metadata));
                        contractId = QString::fromStdString(core_->addContract(std::string{}, strings::toStdString(contractType), std::string{}, actorIds, propertyIds, aliases));
                    }
                }

                transaction.actorId = strings::toStdString(actorId);
                transaction.contractId = strings::toStdString(contractId);
                transaction.allocatable = d.allocatableManualOverride
                                           ? d.allocatable
                                           : (core::importing::contractIsFullyAllocatable(appState, strings::toStdString(contractId)) || d.allocatable);
                transaction.propertyIds = strings::toStdList(d.propertyIds);
                transaction.type = strings::toStdString(d.type);
                input.transactions.push_back(std::move(transaction));
            }

            return QString::fromStdString(core_->finalizeStatementDraft(input));
        });
}

QVariantMap DraftController::currentTransactionViewState(StatementDraft* draft) const
{
    return controllers::guard::invokeValue<QVariantMap>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto* current = ui::currentDraft(draft);
            if (!current) return QVariantMap{};
            return ui::toViewState(core::importing::buildDraftDerivedState(ui::matchingStateForDraft(draft, core_), ui::toCoreSelection(*current)));
        });
}

QVariantMap DraftController::findChoiceRowByText(const QVariantList& rows, const QString& text) const
{
    for (const auto& item : rows) {
        const QVariantMap row = item.toMap();
        if (!row.isEmpty() && rowMatchesText(row, text)) return row;
    }
    return {};
}

void DraftController::syncCurrentTransactionDraft(StatementDraft* draft)
{
    controllers::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
        const auto* current = ui::currentDraft(draft);
        if (!draft || !current) return;

        const auto appState = ui::matchingStateForDraft(draft, core_);
        const auto derived = core::importing::buildDraftDerivedState(appState, ui::toCoreSelection(*current));
        const auto index = draft->currentIndex();
        bool changed = false;

        if (current->actorId.isEmpty()) {
            if (derived.actorCurrentIndex > 0 && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
                const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
                if (!actorRow.synthetic && !actorRow.id.empty()) {
                    const QString actorId = QString::fromStdString(actorRow.id);
                    const QString actorText = QString::fromStdString(actorRow.display.empty() ? actorRow.name : actorRow.display);
                    changed = changed || current->actorId != actorId || current->actorText != actorText || current->newActorSelected;
                    draft->transactions()->setActorId(index, QString::fromStdString(actorRow.id));
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
                         if (const auto* actorRow = ui::findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                            const QString actorText = QString::fromStdString(actorRow->display.empty() ? actorRow->name : actorRow->display);
                            changed = changed || current->actorText != actorText;
                            draft->transactions()->setActorText(index, actorText);
                        }
                        draft->transactions()->setNewActorSelected(index, false);
                    }
                    if (!contractRow.propertyIds.empty()) {
                        const auto propertyIds = ui::toQStringList(contractRow.propertyIds);
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
            const auto propertyIds = ui::toQStringList(derived.autoPropertyIds);
            changed = changed || current->propertyIds != propertyIds;
            draft->transactions()->setProperties(index, propertyIds);
        }

        if (!current->allocatableManualOverride) {
            changed = changed || current->allocatable != derived.effectiveAllocatable;
            draft->transactions()->setAllocatable(index, derived.effectiveAllocatable);
        }

        if (changed) draft->refresh();
    });
}

void DraftController::selectCurrentActorChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setActorId(draft->currentIndex(), QString());
        draft->transactions()->setActorText(draft->currentIndex(), QString());
        draft->transactions()->setNewActorSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setActorId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        draft->transactions()->setActorText(draft->currentIndex(), rowDisplayText(row));
        draft->transactions()->setNewActorSelected(draft->currentIndex(), false);
    }
}

void DraftController::selectCurrentContractChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setContractId(draft->currentIndex(), QString());
        draft->transactions()->setType(draft->currentIndex(), QString());
        draft->transactions()->setNewContractSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setContractId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        const QString type = row.value(QStringLiteral("type")).toString();
        if (!type.isEmpty()) draft->transactions()->setType(draft->currentIndex(), type);

        const auto actorIds = row.value(QStringLiteral("actorIds")).toStringList();
        if (!actorIds.isEmpty()) draft->transactions()->setActorId(draft->currentIndex(), actorIds.first());

        const auto propertyIds = row.value(QStringLiteral("propertyIds")).toStringList();
        if (!propertyIds.isEmpty()) draft->transactions()->setProperties(draft->currentIndex(), propertyIds);

        draft->transactions()->setNewContractSelected(draft->currentIndex(), false);
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
