#include "core/application/workspace/StateHydrator.h"
#include "core/domain/values/AliasUsage.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Transaction.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace {
void dedupStrings(std::vector<std::string>& v) {
    std::unordered_set<std::string> seen;
    std::vector<std::string> out;
    out.reserve(v.size());
    for (const auto& value : v) {
        if (value.empty()) continue;
        if (!seen.insert(value).second) continue;
        out.push_back(value);
    }
    std::sort(out.begin(), out.end());
    v = std::move(out);
}

void dedupAliasUsage(std::vector<core::domain::AliasUsage>& v)
{
    std::unordered_set<std::string> seen;
    std::vector<core::domain::AliasUsage> out;
    out.reserve(v.size());
    for (const auto& value : v) {
        if (value.alias.value.empty()) continue;
        if (!seen.insert(value.alias.value).second) continue;
        out.push_back(value);
    }
    v = std::move(out);
}

void dedupAliases(std::vector<core::domain::Alias>& v)
{
    std::unordered_set<std::string> seen;
    std::vector<core::domain::Alias> out;
    out.reserve(v.size());
    for (const auto& value : v) {
        if (value.value.empty()) continue;
        if (!seen.insert(value.value).second) continue;
        out.push_back(value);
    }
    v = std::move(out);
}
}

namespace core::application {

void StateHydrator::rehydrate(AppState& state) {
    for (auto& a : state.actors) {
        if (!a) continue;
        dedupAliases(a->aliases);
        dedupAliasUsage(a->aliasUsage);
    }
    for (auto& p : state.properties) {
        if (!p) continue;
        dedupAliases(p->aliases);
        dedupAliasUsage(p->aliasUsage);
    }
    for (auto& c : state.contracts) {
        if (!c || c->id.empty()) continue;
        dedupAliases(c->aliases);
        dedupAliasUsage(c->aliasUsage);
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
