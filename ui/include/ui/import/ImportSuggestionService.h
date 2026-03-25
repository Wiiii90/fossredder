/**
 * @file ui/include/ui/import/ImportSuggestionService.h
 * @brief Suggestion heuristics for imported transaction drafts.
 */

#pragma once

#include "core/import/ImportedTransaction.h"
#include "core/models/AppState.h"
#include "ui/models/ImportSuggestion.h"

namespace ui::importing {

/**
 * @brief Builds ranked suggestions for a single imported transaction.
 */
ImportDraftSuggestions buildImportSuggestions(const core::domain::AppState& state,
                                              const ImportedTransaction& transaction);

} // namespace ui::importing
