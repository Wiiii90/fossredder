/**
 * @file core/include/core/import/DraftLinking.h
 * @brief Declares import-draft matching helpers shared by the import pipeline and draft finalization.
 */

#pragma once

#include <string>
#include <vector>

#include "core/import/ImportedTransaction.h"
#include "core/models/AppState.h"

namespace core::importing {

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
    std::string description;
    std::string metadata;
    std::string proofImagePath;
    std::string actorText;
    std::string propertyText;
    std::string actorId;
    bool newActorSelected = false;
    std::string contractId;
    bool newContractSelected = false;
    std::string type;
    bool allocatable = false;
    bool allocatableManualOverride = false;
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

std::string normalizeDraftText(const std::string& text);
bool matchesDraftText(const std::string& left, const std::string& right);
std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata);
DraftTextSignals buildDraftTextSignals(const core::domain::AppState& state,
                                       const ImportedTransaction& transaction);
DraftImportSuggestions buildImportSuggestions(const core::domain::AppState& state,
                                              const ImportedTransaction& transaction);
std::string resolveActorId(const core::domain::AppState& state, const std::string& text);
std::string resolveContractId(const core::domain::AppState& state, const std::string& text);
bool contractIsFullyAllocatable(const core::domain::AppState& state, const std::string& contractId);
DraftDerivedState buildDraftDerivedState(const core::domain::AppState& state,
                                         const DraftLinkSelection& selection);

} // namespace core::importing
