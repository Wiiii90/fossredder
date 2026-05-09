/**
 * @file core/include/core/application/import/draft/DraftMatcher.h
 * @brief Declares import-draft matching helpers shared by the import pipeline and draft finalization.
 */

#pragma once

#include <string>
#include <vector>

#include "core/application/workspace/WorkspaceState.h"
#include "core/application/import/draft/TransactionDraft.h"

namespace core::application::importing::draft {

/**
 * @brief Carries the subset of suggestion data needed for current-draft matching decisions.
 */
struct DraftSuggestionCandidate {
    std::string entityId;
    std::string label;
    std::string entityType;
    std::string sourceText;
    std::string rationale;
    std::string lastUsedAt;
    std::vector<std::string> matchedAliases;
    double confidence = 0.0;
    double score = 0.0;
    double aliasWeight = 0.0;
    double recencyWeight = 0.0;
    int hitCount = 0;
};

/**
 * @brief Groups ranked candidates for one draft link target.
 */
struct DraftSuggestionBucket {
    std::string sourceText;
    std::vector<DraftSuggestionCandidate> candidates;
};

/**
 * @brief Holds core-native ranked suggestions for the import draft.
 */
struct DraftImportSuggestions {
    DraftSuggestionBucket actor;
    DraftSuggestionBucket property;
    DraftSuggestionBucket contract;
};

/**
 * @brief Carries the shared text signals extracted from one imported transaction.
 */
struct DraftTextSignals {
    std::string sharedText;
    std::string actorText;
    std::string propertyText;
    std::string contractText;
    std::string typeText;
};

/**
 * @brief Describes the editable transaction-draft link state independent of any UI toolkit.
 */
struct DraftLinkSelection {
    std::string name;
    std::string metadata;
    std::string actorText;
    std::string propertyText;
    std::string actorId;
    bool actorSelected = false;
    std::string contractId;
    bool contractSelected = false;
    std::string type;
    bool allocatable = false;
    bool allocatableSelected = false;
    std::vector<std::string> propertyIds;
    DraftSuggestionBucket actorSuggestions;
    DraftSuggestionBucket propertySuggestions;
    DraftSuggestionBucket contractSuggestions;
};

/**
 * @brief Represents a selectable catalog row for import-draft assignment UIs.
 */
struct DraftChoiceRow {
    std::string id;
    std::string name;
    std::string display;
    std::string type;
    std::vector<std::string> aliases;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    bool synthetic = false;
    double confidence = 0.0;
    std::string sourceText;
};

/**
 * @brief Contains all derived matching state needed to render the current transaction draft.
 */
struct DraftDerivedState {
    std::string proofSource;
    std::string actorSeedText;
    std::string actorDisplayText;
    std::string contractSeedText;
    std::string contractDisplayText;
    std::string propertySuggestionSummary;
    bool effectiveAllocatable = false;
    int actorCurrentIndex = -1;
    int contractCurrentIndex = -1;
    DraftSuggestionCandidate actorTopSuggestion;
    DraftSuggestionCandidate propertyTopSuggestion;
    DraftSuggestionCandidate contractTopSuggestion;
    bool hasActorTopSuggestion = false;
    bool hasPropertyTopSuggestion = false;
    bool hasContractTopSuggestion = false;
    std::vector<DraftChoiceRow> actorChoices;
    std::vector<DraftChoiceRow> contractChoices;
    std::vector<DraftChoiceRow> propertyRows;
    std::vector<std::string> autoPropertyIds;
};

/**
 * @brief Normalizes free-form draft text for fuzzy matching.
 * @param text Input text to normalize.
 * @return Normalized text suitable for comparisons.
 */
std::string normalizeDraftText(const std::string& text);
/**
 * @brief Compares two draft text fragments after normalization.
 * @param left Left-hand text fragment.
 * @param right Right-hand text fragment.
 * @return True when both fragments normalize to the same value.
 */
bool matchesDraftText(const std::string& left, const std::string& right);
/**
 * @brief Extracts alias-like reference strings from draft metadata.
 * @param metadata Free-form metadata text.
 * @return Extracted reference aliases.
 */
std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata);
/**
 * @brief Derives text signals from a workspace transaction draft.
 * @param state Current workspace state used to enrich the signals.
 * @param transaction Transaction draft to inspect.
 * @return Derived text signals for matching and suggestion generation.
 */
DraftTextSignals buildDraftTextSignals(const core::domain::WorkspaceState& state,
                                       const TransactionDraft& transaction);
/**
 * @brief Builds ranked import suggestions for the given workspace and transaction draft.
 * @param state Current workspace state used as the matching catalog.
 * @param transaction Transaction draft to analyze.
 * @return Ranked import suggestions for actors, properties, and contracts.
 */
DraftImportSuggestions buildImportSuggestions(const core::domain::WorkspaceState& state,
                                              const TransactionDraft& transaction);
/**
 * @brief Resolves the best matching actor identifier for the provided text.
 * @param state Current workspace state used as the actor catalog.
 * @param text Text to resolve.
 * @return Matching actor identifier, or an empty string when no match is found.
 */
std::string resolveActorId(const core::domain::WorkspaceState& state, const std::string& text);
/**
 * @brief Resolves the best matching contract identifier for the provided text.
 * @param state Current workspace state used as the contract catalog.
 * @param text Text to resolve.
 * @return Matching contract identifier, or an empty string when no match is found.
 */
std::string resolveContractId(const core::domain::WorkspaceState& state, const std::string& text);
/**
 * @brief Checks whether all transactions assigned to a contract are allocatable.
 * @param state Current workspace state.
 * @param contractId Contract identifier to inspect.
 * @return True when the contract has at least one transaction and all are allocatable.
 */
bool contractIsFullyAllocatable(const core::domain::WorkspaceState& state, const std::string& contractId);
/**
 * @brief Returns the primary state with missing collections filled from a fallback state.
 * @param primary Primary workspace state to keep when populated.
 * @param fallback Fallback workspace state used to fill gaps.
 * @return A state that prefers primary collections and borrows missing ones from fallback.
 */
core::domain::WorkspaceState withFallbackState(core::domain::WorkspaceState primary,
                                               const core::domain::WorkspaceState& fallback);
/**
 * @brief Derives UI-agnostic matching state for the currently edited draft link.
 * @param state Current workspace state used for catalog lookups.
 * @param selection Current draft link selection.
 * @return Derived selection state ready for presentation or persistence.
 */
DraftDerivedState buildDraftDerivedState(const core::domain::WorkspaceState& state,
                                         const DraftLinkSelection& selection);

} // namespace core::application::importing::draft

namespace core {
namespace importing = application::importing;
}
