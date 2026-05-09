/**
 * @file ui/include/ui/state/WorkspaceClone.h
 * @brief Declarations for the UI workspace clone helpers.
 */

#pragma once

#include <memory>
#include <vector>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/application/workspace/WorkspaceState.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

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

inline core::domain::WorkspaceState cloneWorkspaceState(const core::domain::WorkspaceState& state)
{
    core::domain::WorkspaceState clone;
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
