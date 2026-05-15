/**
 * @file core/tests/application/import/draft/TestDraftMatcherProjection.cpp
 * @brief Tests for draft matcher projection helpers.
 */

#include <gtest/gtest.h>

#include "core/application/import/draft/DraftMatcher.h"

namespace core::application::importing::draft {

TEST(DraftMatcherProjectionTest, BuildsDerivedStateForEmptySelection) {
    core::domain::catalog::WorkspaceCatalog state;
    DraftLinkSelection selection;

    const auto derived = buildDraftDerivedState(state, selection);

    EXPECT_EQ(derived.actorChoices.size(), 1u);
    EXPECT_EQ(derived.contractChoices.size(), 1u);
    EXPECT_TRUE(derived.propertyRows.empty());
    EXPECT_FALSE(derived.effectiveAllocatable);
}

} // namespace core::application::importing::draft
