/**
 * @file ui/include/ui/import/ImportDraftMapper.h
 * @brief Declares helpers that create editable UI draft models from imported transactions.
 */

#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "core/application/workspace/WorkspaceState.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/ports/services/IImportMatcherService.h"

class QObject;
namespace core::domain { class Statement; }

namespace ui {
class StatementDraft;
struct TransactionDraft;
}

namespace ui::importing {

/** @brief Convert imported transactions into editable UI transaction drafts. */
std::vector<TransactionDraft> mapTransactionsToDrafts(const core::domain::WorkspaceState& state,
                                                      const std::vector<core::domain::TransactionDraft>& transactions,
                                                      const std::shared_ptr<core::ports::services::IImportMatcherService>& matcherService = {});

/** @brief Create a statement draft model for an imported statement and its transactions. */
StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const core::domain::WorkspaceState& state,
                                     const std::vector<core::domain::TransactionDraft>& transactions,
                                     const std::shared_ptr<core::ports::services::IImportMatcherService>& matcherService,
                                     const QString& draftId,
                                     int currentTransactionIndex,
                                     QObject* parent);

} // namespace ui::importing
