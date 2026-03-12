/**
 * @file core/tests/unit/TestStateTransforms.cpp
 * @brief Unit tests for `StateHydrator` and `StateProjector` state transformations.
 */

#include "gtest/gtest.h"

#include "core/application/StateHydrator.h"
#include "core/application/StateProjector.h"
#include "core/models/Actor.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <memory>
#include <stdexcept>
#include <vector>

using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Transaction;

namespace {

std::shared_ptr<Contract> makeContract()
{
    auto contract = std::make_shared<Contract>();
    contract->id = "contract-1";
    contract->name = "Lease";
    contract->actorIds = {"actor-2", "actor-1", "actor-1"};
    contract->propertyIds = {"property-2", "property-1", "property-1"};
    return contract;
}

} // namespace

TEST(StateProjectorTests, PrepareForSaveClonesEntitiesAndDeduplicatesContractRelations)
{
    AppState state;
    auto contract = makeContract();
    state.contracts.push_back(contract);

    const auto projected = core::application::StateProjector::prepareForSave(state);

    ASSERT_EQ(projected.contracts.size(), 1u);
    ASSERT_TRUE(projected.contracts.front());
    EXPECT_NE(projected.contracts.front().get(), contract.get());
    EXPECT_EQ(projected.contracts.front()->actorIds,
              (std::vector<std::string>{"actor-1", "actor-2"}));
    EXPECT_EQ(projected.contracts.front()->propertyIds,
              (std::vector<std::string>{"property-1", "property-2"}));

    EXPECT_EQ(contract->actorIds,
              (std::vector<std::string>{"actor-2", "actor-1", "actor-1"}));
    EXPECT_EQ(contract->propertyIds,
              (std::vector<std::string>{"property-2", "property-1", "property-1"}));
}

TEST(StateHydratorTests, RehydrateDeduplicatesContractRelationsInPlace)
{
    AppState state;
    state.contracts.push_back(makeContract());

    core::application::StateHydrator::rehydrate(state);

    ASSERT_EQ(state.contracts.size(), 1u);
    EXPECT_EQ(state.contracts.front()->actorIds,
              (std::vector<std::string>{"actor-1", "actor-2"}));
    EXPECT_EQ(state.contracts.front()->propertyIds,
              (std::vector<std::string>{"property-1", "property-2"}));
}

TEST(StateHydratorTests, ValidateThrowsForUnresolvedTransactionReferencesInStrictMode)
{
    AppState state;

    auto actor = std::make_shared<Actor>();
    actor->id = "actor-1";
    actor->name = "Alice";
    state.actors.push_back(actor);

    auto transaction = std::make_shared<Transaction>();
    transaction->id = "tx-1";
    transaction->name = "Rent";
    transaction->actorId = "actor-1";
    transaction->contractId = "missing-contract";
    state.transactions.push_back(transaction);

    EXPECT_THROW(core::application::StateHydrator::validate(state, true), std::runtime_error);
}
