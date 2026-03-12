#include "core/application/StateHydrator.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace {
void dedupStrings(std::vector<std::string>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}
}

namespace core::application {

void StateHydrator::rehydrate(AppState& state) {
    for (auto& c : state.contracts) {
        if (!c || c->id.empty()) continue;
        dedupStrings(c->actorIds);
        dedupStrings(c->propertyIds);
    }
}

void StateHydrator::validate(const AppState& state, bool strict) {
    const auto fail = [strict](const std::string& msg) {
        if (strict) throw std::runtime_error(msg);
    };
    for (const auto& c : state.contracts) {
        if (!c) continue;
        if (c->name.empty())        fail("Contract.name is empty");
        if (c->actorIds.empty())    fail("Contract has no actors");
        if (c->propertyIds.empty()) fail("Contract has no properties");
    }
    for (const auto& t : state.transactions) {
        if (t && t->name.empty()) fail("Transaction.name is empty");
    }
    std::unordered_set<std::string> actorIds, contractIds;
    for (const auto& a : state.actors)     if (a && !a->id.empty())    actorIds.insert(a->id);
    for (const auto& c : state.contracts)  if (c && !c->id.empty()) contractIds.insert(c->id);
    for (const auto& t : state.transactions) {
        if (!t) continue;
        if (!t->actorId.empty()    && !actorIds.count(t->actorId))    fail("Transaction.actorId unresolved");
        if (!t->contractId.empty() && !contractIds.count(t->contractId)) fail("Transaction.contractId unresolved");
    }
}

} // namespace core::application
