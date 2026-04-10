/**
 * @file core/src/application/DraftFinalizer.cpp
 * @brief Materializes imported draft statements into domain entities.
 */
#include "core/application/DraftFinalizer.h"

#include "core/constants/CoreDefaults.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "../utils/StableId.h"
#include "../utils/Util.h"

#include <charconv>

namespace {

int nextGeneratedContractIndex(const std::vector<std::shared_ptr<Contract>>& contracts)
{
    int maxIdx = 0;
    constexpr auto prefix = core::constants::appState::kGeneratedContractPrefix;
    for (const auto& contractPtr : contracts) {
        if (!contractPtr) continue;
        const std::string& contractName = contractPtr->name;
        if (contractName.size() <= prefix.size() || contractName.rfind(prefix.data(), 0) != 0) continue;
        const std::string rest = contractName.substr(prefix.size());
        int idx = 0;
        const auto result = std::from_chars(rest.data(), rest.data() + rest.size(), idx);
        if (result.ec == std::errc{} && result.ptr == rest.data() + rest.size())
            maxIdx = std::max(maxIdx, idx);
    }
    return maxIdx + 1;
}

}

namespace core::application {

std::string DraftFinalizer::finalize(core::domain::AppState& state, const core::domain::StatementDraft& draft)
{
    if (draft.transactions.empty()) return {};

    auto statement = std::make_shared<Statement>();
    statement->id = core::utils::makeStableId();
    statement->name = ::utils::trim(draft.name).empty()
        ? std::string(core::constants::appState::kDefaultImportedStatementName)
        : draft.name;
    state.statements.push_back(statement);

    for (const auto& item : draft.transactions) {
        auto transaction = std::make_shared<Transaction>();
        transaction->id        = core::utils::makeStableId();
        transaction->name       = item.name;
        transaction->bookingDate = item.bookingDate;
        transaction->amount     = item.amount;
        transaction->description = item.description;
        transaction->statementId = statement->id;
        transaction->status     = item.status;
        transaction->actorId    = item.actorId;
        transaction->contractId = item.contractId;
        transaction->allocatable = item.allocatable;
        transaction->propertyIds = item.propertyIds;
        transaction->valuta.clear();

        const std::string normalizedContractId = ::utils::trim(item.contractId);
        if (!normalizedContractId.empty()) {
            transaction->contractId = normalizedContractId;
        } else {
            const std::string normalizedType = ::utils::trim(item.type);
            if (!normalizedType.empty()) {
                constexpr auto prefix = core::constants::appState::kGeneratedContractPrefix;
                auto contract = std::make_shared<Contract>();
                contract->id = core::utils::makeStableId();
                contract->name = std::string(prefix) + std::to_string(nextGeneratedContractIndex(state.contracts));
                contract->type = normalizedType;
                contract->propertyIds = item.propertyIds;
                state.contracts.push_back(contract);
                transaction->contractId = contract->id;
            }
        }

        state.transactions.push_back(transaction);
    }

    return statement->id;
}

}

