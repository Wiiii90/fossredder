#include "gtest/gtest.h"

#include "core/analysis/Filter.h"
#include "core/models/AppState.h"
#include "core/models/Transaction.h"

TEST(FilterTests, MatchesQuotedDateAndAmountClauses)
{
    auto tx = std::make_shared<Transaction>();
    tx->bookingDate = "2024-05-10";
    tx->amount = 42.5;

    const auto filter = core::analysis::parseFilterSpec("date>='2024-05-01';amount>=40");

    ASSERT_FALSE(filter.empty());
    EXPECT_TRUE(filter.matches(tx, AppState{}));
}

TEST(FilterTests, MatchesUnassignedContractTypeAndProperty)
{
    AppState state;
    auto tx = std::make_shared<Transaction>();
    tx->propertyIds = {"P1"};

    const auto filter = core::analysis::parseFilterSpec("contract.type=unassigned;propertyId=P1");

    ASSERT_FALSE(filter.empty());
    EXPECT_TRUE(filter.matches(tx, state));

    tx->contractId = "C1";
    EXPECT_FALSE(filter.matches(tx, state));
}
