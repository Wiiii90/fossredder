#include "ui/import/ImportDraftMapper.h"

#include <QFileInfo>

#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const std::shared_ptr<Statement>& statement)
{
    std::vector<TransactionDraft> drafts;
    if (!statement) return drafts;

    drafts.reserve(statement->transactions.size());
    for (const auto& tx : statement->transactions) {
        if (!tx) continue;

        TransactionDraft draft;
        draft.name = QString::fromStdString(tx->name);
        draft.bookingDate = QString::fromStdString(tx->bookingDate);
        draft.valuta = QString::fromStdString(tx->valuta);
        draft.amount = tx->amount;
        draft.description = QString::fromStdString(tx->description);
        draft.actorProposal = QString::fromStdString(tx->actorProposal);
        draft.metadata = QString::fromStdString(tx->metadata);
        draft.proofImagePath = QString::fromStdString(tx->proofImagePath);
        draft.status = static_cast<int>(Transaction::Status::Unverified);
        drafts.push_back(std::move(draft));
    }

    return drafts;
}

StatementDraft* createStatementDraft(const QString& sourceFile, const std::shared_ptr<Statement>& statement, QObject* parent)
{
    auto* draft = new StatementDraft(parent);
    draft->setName(QFileInfo(sourceFile).baseName());
    draft->setDrafts(mapTransactionsToDrafts(statement));
    return draft;
}

}