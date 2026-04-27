/**
 * @file core/src/analysis/TransactionCollector.cpp
 * @brief Implements private transaction projection helpers for analysis execution.
 */

#include "TransactionCollector.h"

#include "core/analysis/Filter.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <unordered_map>

namespace core::analysis {

std::vector<AnalysisTransaction> collectTransactionsForAnalysis(const AppState& state,
                                                                const std::string& filterSpec)
{
    std::vector<AnalysisTransaction> out;
    out.reserve(state.transactions.size());

    std::unordered_map<std::string, std::shared_ptr<Contract>> contractById;
    contractById.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        contractById.emplace(contract->id, contract);
    }

    const bool hasFilter = !filterSpec.empty();
    Filter filter;
    if (hasFilter) {
        filter = parseFilterSpec(filterSpec);
    }

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (hasFilter && !filter.matches(transaction, state)) continue;

        AnalysisTransaction projected;
        projected.id = transaction->id;
        projected.name = transaction->name;
        projected.bookingDate = transaction->bookingDate;
        projected.amount = transaction->amount;
        projected.contractId = transaction->contractId;
        projected.propertyIds = transaction->propertyIds;
        if (!transaction->contractId.empty()) {
            const auto it = contractById.find(transaction->contractId);
            if (it != contractById.end() && it->second) projected.contractType = it->second->type;
        }
        out.push_back(std::move(projected));
    }

    return out;
}

}
