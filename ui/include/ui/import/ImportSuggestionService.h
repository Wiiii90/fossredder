/**
 * @file ui/include/ui/import/ImportSuggestionService.h
 * @brief Suggestion heuristics for imported transaction drafts.
 */

#pragma once

#include <memory>

#include "core/ports/services/IImportMatcherService.h"
#include "ui/models/ImportSuggestion.h"

namespace ui::importing {

/**
 * @brief Builds ranked suggestions for a single imported transaction.
 */
ImportDraftSuggestions buildImportSuggestions(const core::ports::services::ImportMatcherPresentation& coreSuggestions);

} // namespace ui::importing
