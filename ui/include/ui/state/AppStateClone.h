#pragma once

#include <memory>
#include <vector>

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

namespace ui {

template <typename T>
std::vector<std::shared_ptr<T>> cloneStateItems(const std::vector<std::shared_ptr<T>>& items)
{
    std::vector<std::shared_ptr<T>> clones;
    clones.reserve(items.size());
    for (const auto& item : items) {
        if (!item) continue;
        clones.push_back(std::make_shared<T>(*item));
    }
    return clones;
}

inline core::domain::AppState cloneAppState(const core::domain::AppState& state)
{
    core::domain::AppState clone;
    clone.actors = cloneStateItems(state.actors);
    clone.properties = cloneStateItems(state.properties);
    clone.contracts = cloneStateItems(state.contracts);
    clone.statements = cloneStateItems(state.statements);
    clone.transactions = cloneStateItems(state.transactions);
    clone.analyses = cloneStateItems(state.analyses);
    clone.annuals = cloneStateItems(state.annuals);
    return clone;
}

}
