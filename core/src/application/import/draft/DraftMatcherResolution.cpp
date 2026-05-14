/**
 * @file core/src/application/import/draft/DraftMatcherResolution.cpp
 * @brief Implements import-draft ID resolution and fallback-state helpers.
 */

#include "core/pch.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/domain/policies/DraftMatchingPolicy.h"

namespace core::application::importing::draft {

namespace policy = core::domain::policies::matching;

std::string resolveActorId(const core::domain::catalog::WorkspaceCatalog& state, const std::string& text)
{
    const auto key = policy::normalizeText(text);
    if (key.empty()) return {};

    for (const auto& actor : state.actors()) {
        if (!actor) continue;
        if (policy::normalizeText(actor->name()) == key) return actor->id();
        for (const auto& alias : actor->aliases()) {
        if (policy::normalizeText(alias.value()) == key) return actor->id();
        }
    }
    return {};
}

std::string resolveContractId(const core::domain::catalog::WorkspaceCatalog& state, const std::string& text)
{
    const auto key = policy::normalizeText(text);
    if (key.empty()) return {};

    for (const auto& contract : state.contracts()) {
        if (!contract) continue;
        if (policy::normalizeText(contract->type()) == key) return contract->id();
        if (policy::normalizeText(contract->name()) == key) return contract->id();
        for (const auto& alias : contract->aliases()) {
            if (policy::normalizeText(alias.value()) == key) return contract->id();
        }
    }
    return {};
}

bool contractIsFullyAllocatable(const core::domain::catalog::WorkspaceCatalog& state, const std::string& contractId)
{
    if (contractId.empty()) return false;

    bool hasTransactions = false;
    for (const auto& transaction : state.transactions()) {
        if (!transaction || transaction->contractId() != contractId) continue;
        hasTransactions = true;
        if (!transaction->isAllocatable()) return false;
    }
    return hasTransactions;
}

core::domain::catalog::WorkspaceCatalog withFallbackState(core::domain::catalog::WorkspaceCatalog primary,
                                                           const core::domain::catalog::WorkspaceCatalog& fallback)
{
    if (primary.actors().empty()) primary.setActors(fallback.actors());
    if (primary.properties().empty()) primary.setProperties(fallback.properties());
    if (primary.contracts().empty()) primary.setContracts(fallback.contracts());
    if (primary.statements().empty()) primary.setStatements(fallback.statements());
    if (primary.transactions().empty()) primary.setTransactions(fallback.transactions());
    if (primary.analyses().empty()) primary.setAnalyses(fallback.analyses());
    if (primary.annuals().empty()) primary.setAnnuals(fallback.annuals());
    return primary;
}

}
