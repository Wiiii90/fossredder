/**
 * @file core/tests/unit/TestDraftLinking.cpp
 * @brief Unit tests for import draft signal extraction and matching state.
 */

#include "gtest/gtest.h"

#include <memory>

#include "core/import/DraftLinking.h"
#include "core/import/ImportedTransaction.h"
#include "core/models/Actor.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"

using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Property;
using core::importing::DraftLinkSelection;

namespace {

std::shared_ptr<Actor> makeActor(const std::string& id, const std::string& name)
{
    auto actor = std::make_shared<Actor>();
    actor->id = id;
    actor->name = name;
    return actor;
}

std::shared_ptr<Property> makeProperty(const std::string& id, const std::string& name)
{
    auto property = std::make_shared<Property>();
    property->id = id;
    property->name = name;
    return property;
}

std::shared_ptr<Contract> makeContract(const std::string& id,
                                       const std::string& name,
                                       const std::string& type,
                                       std::vector<std::string> actorIds = {},
                                       std::vector<std::string> propertyIds = {})
{
    auto contract = std::make_shared<Contract>();
    contract->id = id;
    contract->name = name;
    contract->type = type;
    contract->actorIds = std::move(actorIds);
    contract->propertyIds = std::move(propertyIds);
    return contract;
}

} // namespace

TEST(DraftLinkingTests, ExtractsMultilineActorTextAndKnownTypeFromMetadata)
{
    AppState state;
    state.contracts.push_back(makeContract("contract-type-alpha", "Vertrag 22", "Service"));

    ImportedTransaction tx;
    tx.metadata = "SERVICE-INVOICE-ALPHA\nREFERENCE-TOKEN-ALPHA-0001\nSUMMARY ITEMS:\nOVERVIEW\nTOTAL INCLUDES:\nEUR 10,00 SERVICE FEES";

    const auto signals = core::importing::buildDraftTextSignals(state, tx);

    EXPECT_EQ(signals.sharedText,
              "SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001 SUMMARY ITEMS OVERVIEW TOTAL INCLUDES EUR 10,00 SERVICE FEES");
    EXPECT_EQ(signals.actorText, "SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001");
    EXPECT_EQ(signals.propertyText,
              "SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001 SUMMARY ITEMS OVERVIEW TOTAL INCLUDES EUR 10,00 SERVICE FEES SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001 Service");
    EXPECT_EQ(signals.typeText, "Service");
    EXPECT_EQ(signals.contractText,
              "Service SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001 SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001 SUMMARY ITEMS OVERVIEW TOTAL INCLUDES EUR 10,00 SERVICE FEES");
}

TEST(DraftLinkingTests, StopsActorExtractionBeforeIbanAndReferenceLines)
{
    AppState state;

    ImportedTransaction tx;
    tx.metadata = "ACTOR_SAMPLE_0001\nZZ00SYNTHETICACCOUNT0001\nSALARY_SAMPLE_042025";

    const auto signals = core::importing::buildDraftTextSignals(state, tx);

    EXPECT_EQ(signals.actorText, "ACTOR_SAMPLE_0001");
    EXPECT_TRUE(signals.typeText.empty());
}

TEST(DraftLinkingTests, DoesNotAutoAssignExistingContractFromTypeOnly)
{
    AppState state;
    state.contracts.push_back(makeContract("contract-type-alpha", "Vertrag 22", "Service", {"actor-1"}, {"property-1"}));

    DraftLinkSelection selection;
    selection.type = "Service";

    const auto derived = core::importing::buildDraftDerivedState(state, selection);

    ASSERT_FALSE(derived.contractChoices.empty());
    EXPECT_EQ(derived.contractCurrentIndex, 0);
    EXPECT_TRUE(derived.contractChoices.front().synthetic);
}

TEST(DraftLinkingTests, AutoAssignsExistingContractWhenTypeAndActorMatch)
{
    AppState state;
    state.actors.push_back(makeActor("actor-type-alpha", "SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001"));
    state.contracts.push_back(makeContract("contract-type-alpha", "Vertrag 22", "Service", {"actor-type-alpha"}, {"property-1"}));

    DraftLinkSelection selection;
    selection.actorText = "SERVICE-INVOICE-ALPHA REFERENCE-TOKEN-ALPHA-0001";
    selection.type = "Service";

    const auto derived = core::importing::buildDraftDerivedState(state, selection);

    ASSERT_GT(derived.contractCurrentIndex, 0);
    ASSERT_LT(static_cast<std::size_t>(derived.contractCurrentIndex), derived.contractChoices.size());
    EXPECT_EQ(derived.contractChoices[static_cast<std::size_t>(derived.contractCurrentIndex)].id, "contract-type-alpha");
}

TEST(DraftLinkingTests, ExposesAllPropertyRowsFromState)
{
    AppState state;
    state.properties.push_back(makeProperty("property-1", "Haus A"));
    state.properties.push_back(makeProperty("property-2", "Haus B"));

    DraftLinkSelection selection;
    const auto derived = core::importing::buildDraftDerivedState(state, selection);

    ASSERT_EQ(derived.propertyRows.size(), 2U);
    EXPECT_EQ(derived.propertyRows[0].id, "property-1");
    EXPECT_EQ(derived.propertyRows[1].id, "property-2");
}

TEST(DraftLinkingTests, UsesCoreDerivedPropertyTextForAutoPropertySelection)
{
    AppState state;
    state.properties.push_back(makeProperty("property-1", "Haus A"));

    DraftLinkSelection selection;
    selection.metadata = "ignored";
    selection.propertyText = "Haus A";

    const auto derived = core::importing::buildDraftDerivedState(state, selection);

    ASSERT_EQ(derived.autoPropertyIds.size(), 1U);
    EXPECT_EQ(derived.autoPropertyIds.front(), "property-1");
}
