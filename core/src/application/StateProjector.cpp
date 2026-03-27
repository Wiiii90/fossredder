#include "core/application/StateProjector.h"
#include "core/models/AliasUsage.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include <algorithm>
#include <utility>
#include <unordered_set>
#include <unordered_set>

namespace {

void dedupStrings(std::vector<std::string>& v)
{
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
        if (value.alias.empty()) continue;
        if (!seen.insert(value.alias).second) continue;
        out.push_back(value);
    }
    v = std::move(out);
}

template<typename T>
std::shared_ptr<T> cloneEntity(const std::shared_ptr<T>& e)
{
    return e ? std::make_shared<T>(*e) : nullptr;
}

template<typename Dst, typename Src>
void cloneCollection(Dst& dst, const Src& src)
{
    dst.reserve(src.size());
    for (const auto& item : src)
        dst.push_back(cloneEntity(item));
}

} // namespace

namespace core::application {

AppState StateProjector::prepareForSave(const AppState& state)
{
    AppState out;
    cloneCollection(out.actors,       state.actors);
    cloneCollection(out.properties,   state.properties);
    cloneCollection(out.contracts,    state.contracts);
    cloneCollection(out.statements,   state.statements);
    cloneCollection(out.transactions, state.transactions);
    cloneCollection(out.analyses,     state.analyses);
    cloneCollection(out.annuals,      state.annuals);
    for (const auto& c : out.contracts) {
        if (!c) continue;
        dedupStrings(c->aliases);
        dedupAliasUsage(c->aliasUsage);
        dedupStrings(c->actorIds);
        dedupStrings(c->propertyIds);
    }
    for (const auto& a : out.actors) {
        if (a) {
            dedupStrings(a->aliases);
            dedupAliasUsage(a->aliasUsage);
        }
    }
    for (const auto& p : out.properties) {
        if (p) {
            dedupStrings(p->aliases);
            dedupAliasUsage(p->aliasUsage);
        }
    }
    return out;
}

} // namespace core::application
