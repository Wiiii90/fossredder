/**
 * @file ui/src/import/ImportDraftMapper.cpp
 * @brief Implements helpers that create editable UI draft models from imported transactions.
 */

#include "ui/adapters/core/ImportDraftMapper.h"

#include <QByteArray>
#include <QFileInfo>

#include "core/domain/entities/Statement.h"
#include "ui/adapters/core/ImportSuggestionMapper.h"
#include "ui/adapters/core/DraftViewMapper.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/viewmodels/import/TransactionDraftViewModel.h"

namespace {

void applyInitialDerivedSelections(const core::domain::catalog::WorkspaceCatalog& state, ui::TransactionDraft& draft)
{
    const auto derived = core::application::importing::draft::buildDraftDerivedState(state, ui::importing::toCoreSelection(draft));

    if (draft.actorId.isEmpty()) {
        if (derived.actorCurrentIndex > 0
            && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
            const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
            if (!actorRow.synthetic && !actorRow.id.empty()) {
                draft.actorId = QString::fromStdString(actorRow.id);
                draft.actorText = ui::importing::choiceDisplayText(actorRow);
                draft.actorSelected = false;
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
                    }
                }
                if (draft.propertyIds.isEmpty() && !contractRow.propertyIds.empty()) {
                    draft.propertyIds = ui::importing::toQStringList(contractRow.propertyIds);
                }
                draft.contractSelected = false;
            }
        } else if (draft.type.isEmpty() && !derived.contractSeedText.empty()) {
            draft.type = QString::fromStdString(derived.contractSeedText);
        }
    }

    if (draft.propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
        draft.propertyIds = ui::importing::toQStringList(derived.autoPropertyIds);
    }

    if (draft.contractId.isEmpty() && !draft.contractSelected) {
        draft.type.clear();
    }

    if (!draft.allocatableSelected) {
        draft.allocatable = derived.effectiveAllocatable;
    }
}

} // namespace

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const core::domain::catalog::WorkspaceCatalog& state,
                                                      const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                                                      const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService)
{
    std::vector<TransactionDraft> drafts;

    drafts.reserve(transactions.size());
    for (std::size_t i = 0; i < transactions.size(); ++i) {
        const auto& tx = transactions[i];
        const auto draftSignals = core::application::importing::draft::buildDraftTextSignals(state, tx);
        TransactionDraft draft;
        draft.id = QString::fromStdString(tx.id);
        draft.name = QStringLiteral("Transaction %1").arg(static_cast<int>(i + 1));
        draft.bookingDate = QString::fromStdString(tx.bookingDate);
        draft.valuta = QString::fromStdString(tx.valuta);
        draft.amount = tx.amount;
    draft.actorId = QString::fromStdString(tx.actorId);
    draft.propertyIds = ui::importing::toQStringList(tx.propertyIds);
        draft.actorText = QString::fromStdString(draftSignals.actorText);
        draft.propertyText = QString::fromStdString(draftSignals.propertyText);
        draft.type = QString::fromStdString(draftSignals.typeText);
        draft.metadata = QString::fromStdString(tx.metadata);
        if (!tx.proofImageData.empty()) {
            const QByteArray bytes(reinterpret_cast<const char*>(tx.proofImageData.data()),
                                   static_cast<int>(tx.proofImageData.size()));
            draft.proofImageData = QString::fromLatin1(bytes.toBase64());
        }
        if (matcherService) {
            draft.suggestions = buildImportSuggestions(matcherService->buildImportSuggestions(state, tx));
        }
        drafts.push_back(std::move(draft));
    }

    return drafts;
}

StatementDraft* createStatementDraft(const QString& sourceFile,
                                      const std::shared_ptr<core::domain::Statement>& statement,
                                      const core::domain::catalog::WorkspaceCatalog& state,
                                      const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                                      const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                                      const QString& draftId,
                                      int currentTransactionIndex,
                                      QObject* parent)
{
    auto* draft = new StatementDraft(parent);
    draft->setDraftId(draftId);
    draft->setName((statement && !statement->name().empty())
                       ? QString::fromStdString(statement->name())
                       : QFileInfo(sourceFile).baseName());
    draft->setCatalogState(state);

    auto drafts = mapTransactionsToDrafts(state, transactions, matcherService);
    for (std::size_t i = 0; i < drafts.size() && i < transactions.size(); ++i) {
        applyInitialDerivedSelections(state, drafts[i]);
    }

    draft->setDrafts(std::move(drafts));
    draft->setCurrentIndex(currentTransactionIndex);
    return draft;
}

} // namespace ui::importing

