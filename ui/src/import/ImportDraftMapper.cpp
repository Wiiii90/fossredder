#include "ui/import/ImportDraftMapper.h"

#include <QFileInfo>

#include "core/import/DraftLinking.h"
#include "core/import/ImportedTransaction.h"
#include "core/models/Statement.h"
#include "ui/import/ImportSuggestionService.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"

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
        draft.contractText = QString::fromStdString(draftSignals.contractText);
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
    }

    draft->setDrafts(std::move(drafts));
    return draft;
}

}