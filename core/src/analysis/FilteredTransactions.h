/**
 * @file core/src/analysis/FilteredTransactions.h
 * @brief Declares private helpers for collecting transactions that match an analysis filter.
 */

#pragma once

#include "core/analysis/Filter.h"
#include "core/models/AppState.h"
#include "core/models/Transaction.h"

#include <memory>
#include <string>
#include <vector>

namespace core::analysis::detail {

inline std::vector<std::shared_ptr<Transaction>> collectFilteredTransactions(const AppState& state,
                                                                             const std::string& filterSpec)
{
    Filter filter = parseFilterSpec(filterSpec);
    std::vector<std::shared_ptr<Transaction>> filtered;
    filtered.reserve(state.transactions.size());

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (!filterSpec.empty() && !filter.matches(transaction, state)) continue;
        filtered.push_back(transaction);
    }

    return filtered;
}

} // namespace core::analysis::detail
