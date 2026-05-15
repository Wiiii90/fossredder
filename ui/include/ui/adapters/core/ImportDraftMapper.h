/**
 * @file ui/include/ui/adapters/core/ImportDraftMapper.h
 * @brief Declares helpers that create editable UI draft models from imported transactions.
 */

#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/import/draft/IImportMatcherService.h"

class QObject;
namespace core::domain { class Statement; }

namespace ui {
class StatementDraft;
struct TransactionDraft;
}

namespace ui::importing {

/** @brief Convert imported transactions into editable UI transaction drafts. */
std::vector<TransactionDraft> mapTransactionsToDrafts(const core::domain::catalog::WorkspaceCatalog& state,
                                                      const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                                                      const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService = {});

/** @brief Create a statement draft model for an imported statement and its transactions. */
StatementDraft* createStatementDraft(const QString& sourceFile,
                                     const std::shared_ptr<core::domain::Statement>& statement,
                                     const core::domain::catalog::WorkspaceCatalog& state,
                                     const std::vector<core::application::importing::draft::TransactionDraft>& transactions,
                                     const std::shared_ptr<core::application::importing::draft::IImportMatcherService>& matcherService,
                                     const QString& draftId,
                                     int currentTransactionIndex,
                                     QObject* parent);

} // namespace ui::importing

