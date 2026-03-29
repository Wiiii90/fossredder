/**
 * @file ui/src/controllers/DraftController.cpp
 * @brief Implements the UI controller that finalizes imported statement drafts.
 */

#include "ui/controllers/DraftController.h"

#include "core/application/AppStateFacade.h"
#include "core/import/parsing/AmountParser.h"
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
            const auto input = ui::buildFinalizationInput(draft, core_);
            if (input.transactions.empty()) return QString{};
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
        ui::syncCurrentTransactionDraft(draft, core_);
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
