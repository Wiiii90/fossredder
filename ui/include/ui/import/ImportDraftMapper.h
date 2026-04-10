/**
 * @file ui/include/ui/import/ImportDraftMapper.h
 * @brief Declares helpers that create editable UI draft models from imported transactions.
 */

#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "core/models/AppState.h"
#include "core/models/TransactionDraft.h"

class QObject;
namespace core::domain { class Statement; }

namespace ui {
class StatementDraft;
struct TransactionDraft;
}

namespace ui::importing {

/** @brief Convert imported transactions into editable UI transaction drafts. */
std::vector<TransactionDraft> mapTransactionsToDrafts(const core::domain::AppState& state,
                                                      const std::vector<core::domain::TransactionDraft>& transactions);

/** @brief Create a statement draft model for an imported statement and its transactions. */
StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const core::domain::AppState& state,
                                     const std::vector<core::domain::TransactionDraft>& transactions,
                                     QObject* parent);

} // namespace ui::importing
