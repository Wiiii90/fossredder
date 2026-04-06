/**
 * @file ui/src/import/ImportDraftMapper.cpp
 * @brief Implements helpers that create editable UI draft models from imported transactions.
 */

#include "ui/import/ImportDraftMapper.h"

#include <QFileInfo>

#include "core/import/DraftLinking.h"
#include "core/import/ImportedTransaction.h"
#include "core/models/Statement.h"
#include "ui/import/ImportSuggestionService.h"
#include "ui/import/DraftViewMapper.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"

namespace {

void applyInitialDerivedSelections(const core::domain::AppState& state, ui::TransactionDraft& draft)
{
    const auto derived = core::importing::buildDraftDerivedState(state, ui::importing::toCoreSelection(draft));

    if (draft.actorId.isEmpty()) {
        if (derived.actorCurrentIndex > 0
            && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
            const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
            if (!actorRow.synthetic && !actorRow.id.empty()) {
                draft.actorId = QString::fromStdString(actorRow.id);
                draft.actorText = ui::importing::choiceDisplayText(actorRow);
                draft.newActorSelected = false;
            }
        } else if (draft.actorText.isEmpty() && !derived.actorSeedText.empty()) {
            draft.actorText = QString::fromStdString(derived.actorSeedText);
        }
    }

    if (draft.contractId.isEmpty()) {
        if (derived.contractCurrentIndex > 0
            && static_cast<std::size_t>(derived.contractCurrentIndex) < derived.contractChoices.size()) {
            const auto& contractRow = derived.contractChoices[static_cast<std::size_t>(derived.contractCurrentIndex)];
            if (!contractRow.synthetic && !contractRow.id.empty()) {
                draft.contractId = QString::fromStdString(contractRow.id);
                if (draft.type.isEmpty() && !contractRow.type.empty()) {
                    draft.type = QString::fromStdString(contractRow.type);
                }
                if (!contractRow.actorIds.empty()) {
                    draft.actorId = QString::fromStdString(contractRow.actorIds.front());
                    if (const auto* actorRow = ui::importing::findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                        draft.actorText = ui::importing::choiceDisplayText(*actorRow);
                        draft.newActorSelected = false;
                    }
                }
                if (draft.propertyIds.isEmpty() && !contractRow.propertyIds.empty()) {
                    draft.propertyIds = ui::importing::toQStringList(contractRow.propertyIds);
                }
                draft.newContractSelected = false;
            }
        } else if (draft.type.isEmpty() && !derived.contractSeedText.empty()) {
            draft.type = QString::fromStdString(derived.contractSeedText);
        }
    }

    if (draft.propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
        draft.propertyIds = ui::importing::toQStringList(derived.autoPropertyIds);
    }

    if (draft.contractId.isEmpty() && !draft.newContractSelected) {
        draft.type.clear();
    }

    if (!draft.allocatableManualOverride) {
        draft.allocatable = derived.effectiveAllocatable;
    }
}

} // namespace

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const core::domain::AppState& state,
                                                      const std::vector<ImportedTransaction>& transactions)
{
    std::vector<TransactionDraft> drafts;

    drafts.reserve(transactions.size());
    for (std::size_t i = 0; i < transactions.size(); ++i) {
        const auto& tx = transactions[i];
        const auto draftSignals = core::importing::buildDraftTextSignals(state, tx);
        TransactionDraft draft;
        draft.name = QStringLiteral("Transaction %1").arg(static_cast<int>(i + 1));
        draft.bookingDate = QString::fromStdString(tx.bookingDate);
        draft.valuta = QString::fromStdString(tx.valuta);
        draft.amount = tx.amount;
        draft.description = QString::fromStdString(tx.description);
        draft.actorText = QString::fromStdString(draftSignals.actorText);
        draft.propertyText = QString::fromStdString(draftSignals.propertyText);
        draft.type = QString::fromStdString(draftSignals.typeText);
        draft.metadata = QString::fromStdString(tx.metadata);
        draft.proofImagePath = QString::fromStdString(tx.proofImagePath);
        drafts.push_back(std::move(draft));
    }

    return drafts;
} // namespace ui::importing

StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const core::domain::AppState& state,
                                     const std::vector<ImportedTransaction>& transactions,
                                     QObject* parent)
{
    auto* draft = new StatementDraft(parent);
    draft->setName((statement && !statement->name.empty())
                       ? QString::fromStdString(statement->name)
                       : QFileInfo(sourceFile).baseName());
    draft->setCatalogState(state);

    auto drafts = mapTransactionsToDrafts(state, transactions);
    for (std::size_t i = 0; i < drafts.size() && i < transactions.size(); ++i) {
        drafts[i].suggestions = buildImportSuggestions(state, transactions[i]);
        applyInitialDerivedSelections(state, drafts[i]);
    }

    draft->setDrafts(std::move(drafts));
    return draft;
}

}