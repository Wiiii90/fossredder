/**
 * @file ui/include/ui/import/DraftSessionSupport.h
 * @brief Declares draft-session helpers used by import and draft controller flows.
 */

#pragma once

#include "core/models/AppState.h"
#include "core/models/DraftStatement.h"

namespace core::application { class AppStateFacade; }
namespace ui { class StatementDraft; struct TransactionDraft; }

namespace ui::importing {

/** @brief Return the currently selected transaction draft, if any. */
const TransactionDraft* currentDraft(StatementDraft* draft);

/** @brief Resolve the best available catalog state for a draft session. */
core::domain::AppState matchingStateForDraft(const StatementDraft* draft,
                                             const core::application::AppStateFacade* core);

/** @brief Build the core finalization payload from the current statement draft session. */
core::domain::DraftStatement buildFinalizationInput(StatementDraft* draft,
                                                    core::application::AppStateFacade* core);

/** @brief Synchronize the current transaction draft with derived import suggestions. */
void syncCurrentTransactionDraft(StatementDraft* draft,
                                 core::application::AppStateFacade* core);

} // namespace ui::importing
