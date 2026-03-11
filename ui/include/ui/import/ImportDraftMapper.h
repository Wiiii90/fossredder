#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "core/import/ImportedTransaction.h"

class QObject;
namespace core::domain { class Statement; }

namespace ui {
class StatementDraft;
struct TransactionDraft;
}

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const std::vector<ImportedTransaction>& transactions);
StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const std::vector<ImportedTransaction>& transactions,
                                     QObject* parent);

}