#include "core/application/StateHydrator.h"

#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace {

void dedupStrings(std::vector<std::string>& values)
{
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

void dedupContractRelations(Contract& contract)
{
    dedupStrings(contract.actorIds);
    dedupStrings(contract.propertyIds);
}

}

namespace core::application {

void StateHydrator::rehydrate(AppState& state)
{
    for (auto& contract : state.contracts) {
        if (!contract || contract->id.empty()) continue;
        dedupContractRelations(*contract);
    }
}

void StateHydrator::validate(const AppState& state, bool strictValidation)
{
    auto fail = [strictValidation](const std::string& message) {
        if (strictValidation) throw std::runtime_error(message);
    };

    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        if (contract->name.empty()) fail("Contract.name is empty");
        if (contract->actorIds.empty()) fail("Contract has no actors");
        if (contract->propertyIds.empty()) fail("Contract has no properties");
    }

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (transaction->name.empty()) fail("Transaction.name is empty");
    }

    std::unordered_set<std::string> actorIds;
    actorIds.reserve(state.actors.size());
    for (const auto& actor : state.actors) {
        if (!actor || actor->id.empty()) continue;
        actorIds.insert(actor->id);
    }

    std::unordered_set<std::string> contractIds;
    contractIds.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        if (!contract || contract->id.empty()) continue;
        contractIds.insert(contract->id);
    }

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (!transaction->actorId.empty() && actorIds.find(transaction->actorId) == actorIds.end()) fail("Transaction.actorId unresolved");
        if (!transaction->contractId.empty() && contractIds.find(transaction->contractId) == contractIds.end()) fail("Transaction.contractId unresolved");
    }
}

}
