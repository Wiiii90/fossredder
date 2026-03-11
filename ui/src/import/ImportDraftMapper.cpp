#include "ui/import/ImportDraftMapper.h"

#include <QFileInfo>

#include "core/import/ImportedTransaction.h"
#include "core/models/Statement.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const std::vector<ImportedTransaction>& transactions)
{
    std::vector<TransactionDraft> drafts;

    drafts.reserve(transactions.size());
    for (const auto& tx : transactions) {
        TransactionDraft draft;
        draft.name = QString::fromStdString(tx.name);
        draft.bookingDate = QString::fromStdString(tx.bookingDate);
        draft.valuta = QString::fromStdString(tx.valuta);
        draft.amount = tx.amount;
        draft.description = QString::fromStdString(tx.description);
        draft.metadata = QString::fromStdString(tx.metadata);
        draft.proofImagePath = QString::fromStdString(tx.proofImagePath);
        drafts.push_back(std::move(draft));
    }

    return drafts;
}

StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const std::vector<ImportedTransaction>& transactions,
                                     QObject* parent)
{
    auto* draft = new StatementDraft(parent);
    draft->setName(QFileInfo(sourceFile).baseName());
    draft->setDrafts(mapTransactionsToDrafts(transactions));
    return draft;
}

}