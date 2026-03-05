#pragma once

#include <memory>
#include <vector>

#include <QString>

class QObject;
class Statement;

namespace ui {
class StatementDraft;
struct TransactionDraft;
}

namespace ui::importing {

std::vector<TransactionDraft> mapTransactionsToDrafts(const std::shared_ptr<Statement>& statement);
StatementDraft* createStatementDraft(const QString& sourceFile, const std::shared_ptr<Statement>& statement, QObject* parent);

}