/**
 * @file core/src/analysis/FilteredTransactions.h
 * @brief Declares private helpers for collecting transactions that match an analysis filter.
 */

#pragma once

#include "core/application/analysis/Filter.h"
#include "core/application/workspace/AppState.h"
#include "core/domain/entities/Transaction.h"

#include <memory>
#include <string>
#include <vector>

namespace core::analysis::detail {

inline std::vector<std::shared_ptr<Transaction>> collectFilteredTransactions(const AppState& state,
                                                                             const Filter& filter)
{
    std::vector<std::shared_ptr<Transaction>> filtered;
    filtered.reserve(state.transactions.size());

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (!filter.empty() && !filter.matches(transaction, state)) continue;
        filtered.push_back(transaction);
    }

    return filtered;
}

} // namespace core::analysis::detail
