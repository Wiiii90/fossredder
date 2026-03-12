#include "core/application/StateProjector.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include <algorithm>

namespace {

void dedupStrings(std::vector<std::string>& v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
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
        dedupStrings(c->actorIds);
        dedupStrings(c->propertyIds);
    }
    return out;
}

} // namespace core::application
