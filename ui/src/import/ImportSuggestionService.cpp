/**
 * @file ui/src/import/ImportSuggestionService.cpp
 * @brief Implements alias-based ranking for imported statement drafts.
 */

#include "ui/import/ImportSuggestionService.h"

#include "core/import/DraftLinking.h"

namespace ui::importing {

namespace {

ImportSuggestion toUiSuggestion(const core::importing::DraftSuggestionCandidate& suggestion)
{
    ImportSuggestion out;
    out.entityId = QString::fromStdString(suggestion.entityId);
    out.entityType = QString::fromStdString(suggestion.entityType);
    out.label = QString::fromStdString(suggestion.label);
    out.sourceText = QString::fromStdString(suggestion.sourceText);
    out.rationale = QString::fromStdString(suggestion.rationale);
    out.lastUsedAt = QString::fromStdString(suggestion.lastUsedAt);
    for (const auto& alias : suggestion.matchedAliases) out.matchedAliases.push_back(QString::fromStdString(alias));
    out.score = suggestion.score;
    out.confidence = suggestion.confidence;
    out.aliasWeight = suggestion.aliasWeight;
    out.recencyWeight = suggestion.recencyWeight;
    out.hitCount = suggestion.hitCount;
    return out;
}

ImportSuggestionBucket toUiBucket(const core::importing::DraftSuggestionBucket& bucket)
{
    ImportSuggestionBucket out;
    out.sourceText = QString::fromStdString(bucket.sourceText);
    out.candidates.reserve(static_cast<int>(bucket.candidates.size()));
    for (const auto& suggestion : bucket.candidates) {
        out.candidates.push_back(toUiSuggestion(suggestion));
    }
    return out;
}

} // namespace

ImportDraftSuggestions buildImportSuggestions(const core::domain::AppState& state,
                                              const ImportedTransaction& transaction)
{
    const auto coreSuggestions = core::importing::buildImportSuggestions(state, transaction);
    ImportDraftSuggestions suggestions;
    suggestions.actor = toUiBucket(coreSuggestions.actor);
    suggestions.property = toUiBucket(coreSuggestions.property);
    suggestions.contract = toUiBucket(coreSuggestions.contract);
    return suggestions;
}

} // namespace ui::importing
