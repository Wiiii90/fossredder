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
#include "core/domain/catalog/WorkspaceCatalog.h"
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
        clones.push_back(item);
    }
    return clones;
}

inline core::domain::catalog::WorkspaceCatalog cloneWorkspaceState(const core::domain::catalog::WorkspaceCatalog& state)
{
    core::domain::catalog::WorkspaceCatalog clone;
    clone.setActors(cloneStateItems(state.actors()));
    clone.setProperties(cloneStateItems(state.properties()));
    clone.setContracts(cloneStateItems(state.contracts()));
    clone.setStatements(cloneStateItems(state.statements()));
    clone.setTransactions(cloneStateItems(state.transactions()));
    clone.setAnalyses(cloneStateItems(state.analyses()));
    clone.setAnnuals(cloneStateItems(state.annuals()));
    return clone;
}

}
