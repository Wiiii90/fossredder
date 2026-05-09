/**
 * @file core/src/application/import/draft/DraftMatcherProjection.cpp
 * @brief Implements import-draft projection helpers for derived UI-agnostic state.
 */

#include "core/pch.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/domain/policies/DraftMatchingPolicy.h"

namespace core::application::importing::draft {

namespace policy = core::domain::policies::matching;

namespace {

constexpr double kSyntheticConfidence = 0.35;

bool containsId(const std::vector<std::string>& ids, const std::string& id)
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

bool appendUnique(std::vector<std::string>& values, const std::string& value)
{
    if (value.empty() || containsId(values, value)) return false;
    values.push_back(value);
    return true;
}

template <typename TEntity>
std::vector<core::domain::AliasUsage> aliasUsages(const TEntity& entity)
{
    if (!entity.aliasUsage.empty()) return entity.aliasUsage;

    std::vector<core::domain::AliasUsage> out;
    out.reserve(entity.aliases.size());
    for (const auto& alias : entity.aliases) {
        if (alias.value.empty()) continue;
        core::domain::AliasUsage usage;
        usage.alias = alias;
        usage.hitCount = 1;
        usage.createdAt = alias.createdAt;
        usage.updatedAt = alias.updatedAt;
        out.push_back(std::move(usage));
    }
    return out;
}

const DraftSuggestionCandidate* topSuggestion(const DraftSuggestionBucket& bucket)
{
    return bucket.candidates.empty() ? nullptr : &bucket.candidates.front();
}

int confidencePercent(const DraftSuggestionCandidate* suggestion)
{
    return suggestion ? static_cast<int>(std::round(std::clamp(suggestion->confidence, 0.0, 1.0) * 100.0)) : 0;
}

std::string actorDisplay(const core::domain::Actor& actor)
{
    return actor.name;
}

std::string propertyDisplay(const core::domain::Property& property)
{
    return property.name;
}

DraftChoiceRow actorRow(const std::shared_ptr<core::domain::Actor>& actor)
{
    DraftChoiceRow row;
    if (!actor) return row;
    row.id = actor->id;
    row.name = actor->name;
    row.display = actorDisplay(*actor);
    for (const auto& alias : actor->aliases) {
        row.aliases.push_back(alias.value);
    }
    return row;
}

DraftChoiceRow propertyRow(const std::shared_ptr<core::domain::Property>& property)
{
    DraftChoiceRow row;
    if (!property) return row;
    row.id = property->id;
    row.name = property->name;
    row.display = propertyDisplay(*property);
    for (const auto& alias : property->aliases) {
        row.aliases.push_back(alias.value);
    }
    return row;
}

DraftChoiceRow contractRow(const std::shared_ptr<core::domain::Contract>& contract)
{
    DraftChoiceRow row;
    if (!contract) return row;
    row.id = contract->id;
    row.name = contract->name;
    row.display = contract->name;
    row.type = contract->type;
    for (const auto& alias : contract->aliases) {
        row.aliases.push_back(alias.value);
    }
    row.actorIds = contract->actorIds;
    row.propertyIds = contract->propertyIds;
    return row;
}

std::vector<DraftChoiceRow> actorRows(const core::domain::WorkspaceState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.actors.size());
    for (const auto& actor : state.actors) {
        auto row = actorRow(actor);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<DraftChoiceRow> propertyRows(const core::domain::WorkspaceState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.properties.size());
    for (const auto& property : state.properties) {
        auto row = propertyRow(property);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<DraftChoiceRow> contractRows(const core::domain::WorkspaceState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        auto row = contractRow(contract);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

int rowIndexById(const std::vector<DraftChoiceRow>& rows, const std::string& id)
{
    if (id.empty()) return -1;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].id == id) return static_cast<int>(i);
    }
    return -1;
}

const DraftChoiceRow* rowByIndex(const std::vector<DraftChoiceRow>& rows, int index)
{
    return (index >= 0 && static_cast<std::size_t>(index) < rows.size()) ? &rows[static_cast<std::size_t>(index)] : nullptr;
}

std::string firstExistingText(const DraftChoiceRow& row)
{
    if (!row.display.empty()) return row.display;
    if (!row.name.empty()) return row.name;
    if (!row.type.empty()) return row.type;
    return row.id;
}

bool rowMatchesText(const DraftChoiceRow& row, const std::string& text)
{
    const auto key = policy::normalizeText(text);
    if (key.empty()) return false;

    if (policy::normalizeText(row.name) == key) return true;
    if (policy::normalizeText(row.display) == key) return true;
    if (policy::normalizeText(row.type) == key) return true;
    for (const auto& alias : row.aliases) {
        if (policy::normalizeText(alias) == key) return true;
    }
    return false;
}

const DraftChoiceRow* findRowByText(const std::vector<DraftChoiceRow>& rows, const std::string& text)
{
    for (const auto& row : rows) {
        if (rowMatchesText(row, text)) return &row;
    }
    return nullptr;
}

std::string actorDisplayText(const DraftLinkSelection& selection,
                             const std::vector<DraftChoiceRow>& rows,
                             const DraftSuggestionCandidate* top)
{
    if (!selection.actorId.empty()) {
        const auto index = rowIndexById(rows, selection.actorId);
        const auto* row = rowByIndex(rows, index);
        return row ? firstExistingText(*row) : selection.actorId;
    }
    if (top && !top->label.empty()) return top->label;
    const auto actorText = policy::trim(selection.actorText);
    if (!actorText.empty()) return actorText;
    return {};
}

std::string contractDisplayText(const DraftLinkSelection& selection,
                                const std::vector<DraftChoiceRow>& rows,
                                const DraftSuggestionCandidate* top)
{
    if (!selection.contractId.empty()) {
        const auto index = rowIndexById(rows, selection.contractId);
        const auto* row = rowByIndex(rows, index);
        if (row) {
            if (!row->name.empty()) return row->name;
            return firstExistingText(*row);
        }
        return selection.contractId;
    }
    (void)top;
    return {};
}

std::string actorSeedText(const DraftLinkSelection& selection,
                          const std::string& displayText,
                          const DraftSuggestionCandidate* top)
{
    if (!selection.actorId.empty()) return displayText;
    const auto actorText = policy::trim(selection.actorText);
    if (selection.actorSelected && actorText.empty()) return {};
    if (!actorText.empty()) return actorText;
    return top ? top->label : std::string{};
}

std::string contractSeedText(const DraftLinkSelection& selection,
                             const std::string& displayText,
                             const DraftSuggestionCandidate* top)
{
    if (!selection.contractId.empty()) return displayText;
    const auto type = policy::trim(selection.type);
    if (selection.contractSelected && type.empty()) return {};
    if (!type.empty()) return type;
    return {};
}

std::string propertySuggestionSummary(const DraftSuggestionBucket& bucket)
{
    const auto* top = topSuggestion(bucket);
    if (!top) return "No property suggestion";

    std::vector<std::string> labels;
    const auto maxCount = std::min<std::size_t>(2, bucket.candidates.size());
    labels.reserve(maxCount);
    for (std::size_t i = 0; i < maxCount; ++i) {
        if (!bucket.candidates[i].label.empty()) labels.push_back(bucket.candidates[i].label);
    }

    return "Confidence: " + std::to_string(confidencePercent(top)) + "% (" + policy::joinNonEmptyLines(labels, ", ") + ")";
}

std::vector<std::string> propertyAutoSelectIds(const DraftLinkSelection& selection,
                                               const std::vector<DraftChoiceRow>& properties,
                                               const std::vector<DraftChoiceRow>& contracts,
                                               const std::string& selectedContractId)
{
    std::vector<std::string> ids;
    const std::string source = !policy::trim(selection.propertyText).empty()
        ? selection.propertyText
        : selection.metadata + " " + selection.actorText + " " + selection.type;
    const auto containsNormalized = [](const std::string& haystack, const std::string& needle) {
        const auto h = policy::normalizeText(haystack);
        const auto n = policy::normalizeText(needle);
        return !h.empty() && !n.empty() && h.find(n) != std::string::npos;
    };

    for (const auto& row : properties) {
        if (row.id.empty()) continue;
        if (containsNormalized(source, row.name) || containsNormalized(source, row.display)) {
            appendUnique(ids, row.id);
            continue;
        }
        for (const auto& alias : row.aliases) {
            if (containsNormalized(source, alias)) {
                appendUnique(ids, row.id);
                break;
            }
        }
    }

    for (const auto& suggestion : selection.propertySuggestions.candidates) {
        if (suggestion.confidence < 0.25) continue;
        for (const auto& row : properties) {
            if (row.id.empty()) continue;
            if (row.id == suggestion.entityId || containsNormalized(suggestion.label, row.name) || containsNormalized(suggestion.label, row.display)) {
                appendUnique(ids, row.id);
                break;
            }
        }
    }

    const auto contractId = !selectedContractId.empty() ? selectedContractId : selection.contractId;
    if (!contractId.empty()) {
        const auto contractIndex = rowIndexById(contracts, contractId);
        if (const auto* contractRow = rowByIndex(contracts, contractIndex)) {
            for (const auto& propertyId : contractRow->propertyIds) appendUnique(ids, propertyId);
        }
    }

    return ids;
}

double contractScore(const DraftLinkSelection& selection,
                     const DraftChoiceRow& row,
                     const std::vector<DraftChoiceRow>& actors,
                     const DraftSuggestionCandidate* topContract,
                     const std::string& actorDisplay)
{
    double score = 0.0;
    const auto typeText = policy::trim(selection.type);
    const auto actorText = policy::trim(selection.actorText).empty() ? actorDisplay : policy::trim(selection.actorText);

    if (!typeText.empty()) {
        if (policy::matchesDraftText(row.type, typeText)) score += 600.0;
        if (policy::matchesDraftText(row.name, typeText)) score += 420.0;
        if (policy::matchesDraftText(row.display, typeText)) score += 360.0;
    }

    if (!actorText.empty()) {
        for (const auto& actorId : row.actorIds) {
            const auto actorIndex = rowIndexById(actors, actorId);
            const auto* actorRowPtr = rowByIndex(actors, actorIndex);
            const auto actorName = actorRowPtr ? firstExistingText(*actorRowPtr) : actorId;
            if (policy::matchesDraftText(actorName, actorText)) {
                score += 520.0;
                break;
            }
        }
        if (policy::matchesDraftText(row.name, actorText) || policy::matchesDraftText(row.type, actorText)) score += 180.0;
    }

    for (const auto& propertyId : selection.propertyIds) {
        if (containsId(row.propertyIds, propertyId)) {
            score += 160.0;
            break;
        }
    }

    if (topContract && row.id == topContract->entityId) score += 40.0;
    return score;
}

template <typename EntityRange, typename LabelFn, typename TextFn>
DraftSuggestionBucket buildSuggestionBucket(const EntityRange& entities,
                                            const std::string& entityType,
                                            LabelFn&& labelFn,
                                            TextFn&& textFn,
                                            const std::string& sourceText)
{
    DraftSuggestionBucket bucket;
    bucket.sourceText = sourceText;

    const auto sourceTokens = policy::tokens(sourceText);
    const auto sourceLeadTokens = policy::tokens(policy::leadingText(sourceText, 4));

    struct RankedSuggestion {
        DraftSuggestionCandidate value;
    };
    std::vector<RankedSuggestion> ranked;
    ranked.reserve(entities.size());

    for (const auto& entity : entities) {
        if (!entity) continue;

        DraftSuggestionCandidate suggestion;
        suggestion.entityType = entityType;
        suggestion.entityId = entity->id;
        suggestion.label = labelFn(*entity);
        suggestion.sourceText = sourceText;

        const auto candidateText = policy::normalizeText(textFn(*entity));
        const auto candidateTokens = policy::tokens(textFn(*entity));
        const auto usages = aliasUsages(*entity);

        double score = 0.0;
        std::vector<std::string> matchedAliases;
        double aliasScore = 0.0;
        double recencyScore = 0.0;
        int bestHitCount = 0;
        std::string bestLastUsedAt;

        double strongestUsageFactor = 0.0;
        for (const auto& usage : usages) {
            strongestUsageFactor = std::max(strongestUsageFactor,
                                            policy::aliasHitWeight(usage.hitCount) * 0.65 + policy::aliasRecencyWeight(usage.lastUsedAt) * 0.35);
        }
        if (strongestUsageFactor <= 0.0) strongestUsageFactor = 0.15;

        if (!candidateText.empty() && normalizeDraftText(sourceText).find(candidateText) != std::string::npos) {
            score += 40.0 + (40.0 * strongestUsageFactor * 0.2);
        }
        score += policy::tokenOverlapScore(sourceTokens, candidateTokens) * (12.0 + strongestUsageFactor * 4.0);
        score += policy::tokenOverlapScore(sourceLeadTokens, candidateTokens) * 18.0;

        for (const auto& usage : usages) {
            const std::string alias = usage.alias.value;
            const auto aliasNorm = policy::normalizeText(alias);
            if (aliasNorm.empty()) continue;

            const double hitWeight = policy::aliasHitWeight(usage.hitCount);
            const double recentWeight = policy::aliasRecencyWeight(usage.lastUsedAt);
            const double usageFactor = 1.0 + hitWeight + recentWeight * 0.75;

            if (normalizeDraftText(sourceText).find(aliasNorm) != std::string::npos) {
                score += 80.0 * usageFactor;
                matchedAliases.push_back(alias);
                aliasScore += hitWeight;
                recencyScore += recentWeight;
                bestHitCount = std::max(bestHitCount, usage.hitCount);
                if (!usage.lastUsedAt.empty() && (bestLastUsedAt.empty() || usage.lastUsedAt > bestLastUsedAt)) bestLastUsedAt = usage.lastUsedAt;
                continue;
            }

            const auto aliasTokens = policy::tokens(alias);
            const int aliasOverlap = policy::tokenOverlapScore(sourceTokens, aliasTokens);
            if (aliasOverlap > 0) {
                score += (10.0 * aliasOverlap) * (1.0 + hitWeight * 0.5 + recentWeight * 0.25);
                matchedAliases.push_back(alias);
                aliasScore += hitWeight * 0.5;
                recencyScore += recentWeight * 0.5;
                bestHitCount = std::max(bestHitCount, usage.hitCount);
                if (!usage.lastUsedAt.empty() && (bestLastUsedAt.empty() || usage.lastUsedAt > bestLastUsedAt)) bestLastUsedAt = usage.lastUsedAt;
            }
        }

        if (score <= 0.0) {
            score = 0.05;
            suggestion.rationale = "Fallback";
        }

        suggestion.score = score;
        suggestion.confidence = std::clamp(score / 220.0, 0.0, 1.0);
        suggestion.aliasWeight = std::clamp(aliasScore, 0.0, 1.0);
        suggestion.recencyWeight = std::clamp(recencyScore, 0.0, 1.0);
        suggestion.hitCount = bestHitCount;
        suggestion.lastUsedAt = bestLastUsedAt;
        suggestion.matchedAliases = matchedAliases;
        if (!matchedAliases.empty()) {
            suggestion.rationale = "Alias match";
        } else if (!candidateText.empty() && normalizeDraftText(sourceText).find(candidateText) != std::string::npos) {
            suggestion.rationale = "Name match";
        } else {
            suggestion.rationale = "Token overlap";
        }

        ranked.push_back({std::move(suggestion)});
    }

    std::sort(ranked.begin(), ranked.end(), [](const RankedSuggestion& lhs, const RankedSuggestion& rhs) {
        if (lhs.value.score == rhs.value.score) {
            if (lhs.value.confidence == rhs.value.confidence) return lhs.value.label < rhs.value.label;
            return lhs.value.confidence > rhs.value.confidence;
        }
        return lhs.value.score > rhs.value.score;
    });

    constexpr std::size_t kMaxCandidates = 5;
    bucket.candidates.reserve(std::min(kMaxCandidates, ranked.size()));
    for (std::size_t i = 0; i < ranked.size() && i < kMaxCandidates; ++i) {
        bucket.candidates.push_back(std::move(ranked[i].value));
    }

    return bucket;
}

} // namespace

DraftDerivedState buildDraftDerivedState(const core::domain::WorkspaceState& state,
                                         const DraftLinkSelection& selection)
{
    auto effectiveSelection = selection;
    if (policy::trim(effectiveSelection.type).empty() && !policy::trim(effectiveSelection.metadata).empty()) {
        const auto metadataLines = policy::splitLines(effectiveSelection.metadata);
        effectiveSelection.type = policy::extractTypeText(state, effectiveSelection.metadata, metadataLines);
    }

    DraftDerivedState derived;
    const auto actors = actorRows(state);
    const auto properties = propertyRows(state);
    const auto contracts = contractRows(state);

    derived.propertyRows = properties;
    derived.proofSource.clear();

    const auto* actorTop = topSuggestion(effectiveSelection.actorSuggestions);
    const auto* propertyTop = topSuggestion(effectiveSelection.propertySuggestions);
    const auto* contractTop = topSuggestion(effectiveSelection.contractSuggestions);
    if (actorTop) {
        derived.actorTopSuggestion = *actorTop;
        derived.hasActorTopSuggestion = true;
    }
    if (propertyTop) {
        derived.propertyTopSuggestion = *propertyTop;
        derived.hasPropertyTopSuggestion = true;
    }
    if (contractTop) {
        derived.contractTopSuggestion = *contractTop;
        derived.hasContractTopSuggestion = true;
    }

    derived.actorDisplayText = actorDisplayText(effectiveSelection, actors, actorTop);
    derived.contractDisplayText = contractDisplayText(effectiveSelection, contracts, contractTop);
    derived.actorSeedText = actorSeedText(effectiveSelection, derived.actorDisplayText, actorTop);
    derived.contractSeedText = contractSeedText(effectiveSelection, derived.contractDisplayText, contractTop);
    derived.propertySuggestionSummary = propertySuggestionSummary(effectiveSelection.propertySuggestions);
    derived.effectiveAllocatable = effectiveSelection.allocatableSelected
                                      ? effectiveSelection.allocatable
                                      : (core::application::importing::draft::contractIsFullyAllocatable(state, effectiveSelection.contractId) || effectiveSelection.allocatable);

    derived.actorChoices = actors;
    DraftChoiceRow newActor;
    newActor.name = "New Actor";
    newActor.display = "New Actor";
    newActor.synthetic = true;
    newActor.confidence = actorTop ? actorTop->confidence : kSyntheticConfidence;
    newActor.sourceText = derived.actorSeedText;
    derived.actorChoices.insert(derived.actorChoices.begin(), std::move(newActor));

    std::vector<std::pair<double, DraftChoiceRow>> scoredContracts;
    scoredContracts.reserve(contracts.size());
    for (const auto& row : contracts) {
        scoredContracts.push_back({contractScore(effectiveSelection, row, actors, contractTop, derived.actorDisplayText), row});
    }
    std::sort(scoredContracts.begin(), scoredContracts.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first == rhs.first) return lhs.second.display < rhs.second.display;
        return lhs.first > rhs.first;
    });

    DraftChoiceRow newContract;
    newContract.name = "New Contract";
    newContract.display = "New Contract";
    newContract.type = derived.contractSeedText;
    newContract.synthetic = true;
    newContract.confidence = contractTop ? contractTop->confidence : kSyntheticConfidence;
    newContract.sourceText = derived.contractSeedText;
    derived.contractChoices.push_back(std::move(newContract));
    for (const auto& [score, row] : scoredContracts) {
        (void)score;
        derived.contractChoices.push_back(row);
    }

    if (!effectiveSelection.actorId.empty()) {
        derived.actorCurrentIndex = rowIndexById(derived.actorChoices, effectiveSelection.actorId);
    } else if (effectiveSelection.actorSelected) {
        derived.actorCurrentIndex = 0;
    } else if (actorTop && !actorTop->entityId.empty() && actorTop->confidence >= 0.2) {
        derived.actorCurrentIndex = rowIndexById(derived.actorChoices, actorTop->entityId);
    } else if (!derived.actorSeedText.empty()) {
        if (const auto* match = findRowByText(actors, derived.actorSeedText)) {
            derived.actorCurrentIndex = rowIndexById(derived.actorChoices, match->id);
        }
    }
    if (derived.actorCurrentIndex < 0) derived.actorCurrentIndex = derived.actorChoices.empty() ? -1 : 0;

    if (!effectiveSelection.contractId.empty()) {
        derived.contractCurrentIndex = rowIndexById(derived.contractChoices, effectiveSelection.contractId);
    } else if (effectiveSelection.contractSelected) {
        derived.contractCurrentIndex = 0;
    } else if (!scoredContracts.empty() && scoredContracts.front().first >= 900.0) {
        derived.contractCurrentIndex = rowIndexById(derived.contractChoices, scoredContracts.front().second.id);
    } else if (!derived.contractSeedText.empty()
               && (!policy::trim(effectiveSelection.actorText).empty()
                   || !effectiveSelection.actorId.empty()
                   || !effectiveSelection.propertyIds.empty())) {
        if (const auto* match = findRowByText(contracts, derived.contractSeedText)) {
            derived.contractCurrentIndex = rowIndexById(derived.contractChoices, match->id);
        }
    }
    if (derived.contractCurrentIndex < 0) derived.contractCurrentIndex = derived.contractChoices.empty() ? -1 : 0;

    std::string selectedContractId = effectiveSelection.contractId;
    if (selectedContractId.empty() && derived.contractCurrentIndex > 0) {
        if (const auto* contractRow = rowByIndex(derived.contractChoices, derived.contractCurrentIndex);
            contractRow && !contractRow->synthetic) {
            selectedContractId = contractRow->id;
        }
    }
    derived.autoPropertyIds = propertyAutoSelectIds(effectiveSelection, properties, contracts, selectedContractId);

    return derived;
}

DraftImportSuggestions buildImportSuggestions(const core::domain::WorkspaceState& state,
                                              const core::domain::TransactionDraft& transaction)
{
    const auto signals = buildDraftTextSignals(state, transaction);

    const auto actorSourceText = policy::trim(policy::joinNonEmptyLines({signals.actorText, signals.sharedText}, " "));
    const auto& propertySourceText = signals.propertyText;
    const auto contractSourceText = signals.sharedText;

    DraftImportSuggestions suggestions;
    suggestions.actor = buildSuggestionBucket(state.actors,
                                              "actor",
                                              [](const core::domain::Actor& actor) {
                                                  return actor.name;
                                              },
                                              [](const core::domain::Actor& actor) {
                                                  return actor.name;
                                              },
                                              actorSourceText);

    suggestions.property = buildSuggestionBucket(state.properties,
                                                 "property",
                                                 [](const core::domain::Property& property) {
                                                     return property.name;
                                                 },
                                                 [](const core::domain::Property& property) {
                                                     return property.name;
                                                 },
                                                 propertySourceText);

    suggestions.contract = buildSuggestionBucket(state.contracts,
                                                 "contract",
                                                 [](const core::domain::Contract& contract) {
                                                     if (contract.name.empty()) return contract.type;
                                                     if (contract.type.empty()) return contract.name;
                                                     return contract.type + " — " + contract.name;
                                                 },
                                                 [](const core::domain::Contract& contract) {
                                                     return contract.type.empty() ? contract.name : contract.type + " " + contract.name;
                                                 },
                                                 contractSourceText);

    return suggestions;
}

} // namespace core::application::importing::draft
