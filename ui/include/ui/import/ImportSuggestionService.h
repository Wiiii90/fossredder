/**
 * @file ui/include/ui/import/ImportSuggestionService.h
 * @brief Suggestion heuristics for imported transaction drafts.
 */

#pragma once

#include <memory>

#include "core/application/import/draft/IImportMatcherService.h"
#include "ui/models/ImportSuggestion.h"

namespace ui::importing {

/**
 * @brief Builds ranked suggestions for a single imported transaction.
 */
ImportDraftSuggestions buildImportSuggestions(const core::application::importing::draft::ImportMatcherPresentation& coreSuggestions);

} // namespace ui::importing
