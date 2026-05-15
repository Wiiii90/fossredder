/**
 * @file core/tests/application/import/draft/TestDraftMatcher.cpp
 * @brief Tests for draft matching helpers.
 */

#include <gtest/gtest.h>

#include "core/application/import/draft/DraftMatcher.h"

namespace core::application::importing::draft {

TEST(DraftMatcherTest, ResolvesActorAndContractIdsFromWorkspaceState) {
    core::domain::catalog::WorkspaceCatalog state;

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alpha Haus");
    actor->addAlias(core::domain::Alias{"Alpha", "kind", "Alpha", "created", "updated", 1, "last"});
    state.setActors({actor});

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Lease");
    contract->setType("rent");
    contract->addAlias(core::domain::Alias{"Rent", "kind", "Rent", "created", "updated", 1, "last"});
    contract->setActorIds({"actor-1"});
    state.setContracts({contract});

    EXPECT_EQ(resolveActorId(state, " Alpha "), "actor-1");
    EXPECT_EQ(resolveContractId(state, " rent "), "contract-1");
    EXPECT_TRUE(contractIsFullyAllocatable(state, "contract-1") == false);
}

TEST(DraftMatcherTest, NormalizesTextAndFillsFallbackState) {
    core::domain::catalog::WorkspaceCatalog primary;
    core::domain::catalog::WorkspaceCatalog fallback;

    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Alpha");
    fallback.setActors({actor});

    const auto merged = withFallbackState(primary, fallback);

    EXPECT_EQ(merged.actors().size(), 1u);
    EXPECT_EQ(normalizeDraftText("Alpha-Beta"), "alpha beta");
    EXPECT_TRUE(matchesDraftText("Alpha", " alpha "));
}

} // namespace core::application::importing::draft
