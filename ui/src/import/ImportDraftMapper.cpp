#include "ui/import/ImportDraftMapper.h"

#include <QFileInfo>

#include "core/import/DraftLinking.h"
#include "core/import/ImportedTransaction.h"
#include "core/models/Statement.h"
#include "ui/import/ImportSuggestionService.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"

namespace {

core::importing::DraftSuggestionCandidate toCoreCandidate(const ui::ImportSuggestion& suggestion)
{
    core::importing::DraftSuggestionCandidate out;
    out.entityId = suggestion.entityId.toStdString();
    out.label = suggestion.label.toStdString();
    out.confidence = suggestion.confidence;
    return out;
}

core::importing::DraftSuggestionBucket toCoreBucket(const ui::ImportSuggestionBucket& bucket)
{
    core::importing::DraftSuggestionBucket out;
    out.candidates.reserve(bucket.candidates.size());
    for (const auto& suggestion : bucket.candidates) {
        out.candidates.push_back(toCoreCandidate(suggestion));
    }
    return out;
}

core::importing::DraftLinkSelection toCoreSelection(const ui::TransactionDraft& draft)
{
    core::importing::DraftLinkSelection out;
    out.name = draft.name.toStdString();
    out.description = draft.description.toStdString();
    out.metadata = draft.metadata.toStdString();
    out.proofImagePath = draft.proofImagePath.toStdString();
    out.actorText = draft.actorText.toStdString();
    out.propertyText = draft.propertyText.toStdString();
    out.actorId = draft.actorId.toStdString();
    out.newActorSelected = draft.newActorSelected;
    out.contractId = draft.contractId.toStdString();
    out.newContractSelected = draft.newContractSelected;
    out.type = draft.type.toStdString();
    out.allocatable = draft.allocatable;
    out.allocatableManualOverride = draft.allocatableManualOverride;
    for (const auto& propertyId : draft.propertyIds) {
        out.propertyIds.push_back(propertyId.toStdString());
    }
    out.actorSuggestions = toCoreBucket(draft.suggestions.actor);
    out.propertySuggestions = toCoreBucket(draft.suggestions.property);
    out.contractSuggestions = toCoreBucket(draft.suggestions.contract);
    return out;
}

QStringList toQStringList(const std::vector<std::string>& values)
{
    QStringList out;
    out.reserve(static_cast<int>(values.size()));
    for (const auto& value : values) {
        out.push_back(QString::fromStdString(value));
    }
    return out;
}

const core::importing::DraftChoiceRow* findChoiceRowById(
    const std::vector<core::importing::DraftChoiceRow>& rows, const std::string& id)
{
    if (id.empty()) return nullptr;
    for (const auto& row : rows) {
        if (row.id == id) return &row;
    }
    return nullptr;
}

QString choiceDisplayText(const core::importing::DraftChoiceRow& row)
{
    if (!row.display.empty()) return QString::fromStdString(row.display);
    if (!row.name.empty()) return QString::fromStdString(row.name);
    if (!row.type.empty()) return QString::fromStdString(row.type);
    return QString::fromStdString(row.id);
}

void applyInitialDerivedSelections(const core::domain::AppState& state, ui::TransactionDraft& draft)
{
    const auto derived = core::importing::buildDraftDerivedState(state, toCoreSelection(draft));

    if (draft.actorId.isEmpty()) {
        if (derived.actorCurrentIndex > 0
            && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
            const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
            if (!actorRow.synthetic && !actorRow.id.empty()) {
                draft.actorId = QString::fromStdString(actorRow.id);
                draft.actorText = choiceDisplayText(actorRow);
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
                    if (const auto* actorRow = findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                        draft.actorText = choiceDisplayText(*actorRow);
                        draft.newActorSelected = false;
                    }
                }
                if (draft.propertyIds.isEmpty() && !contractRow.propertyIds.empty()) {
                    draft.propertyIds = toQStringList(contractRow.propertyIds);
                }
                draft.newContractSelected = false;
            }
        } else if (draft.type.isEmpty() && !derived.contractSeedText.empty()) {
            draft.type = QString::fromStdString(derived.contractSeedText);
        }
    }

    if (draft.propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
        draft.propertyIds = toQStringList(derived.autoPropertyIds);
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
}

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