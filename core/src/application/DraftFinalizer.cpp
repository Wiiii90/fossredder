#include "core/application/DraftFinalizer.h"

#include "core/constants/CoreDefaults.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/utils/StableId.h"

#include <algorithm>
#include <charconv>
#include <cctype>

namespace {

std::string trimCopy(const std::string& value)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    const auto begin = std::find_if_not(value.begin(), value.end(), isSpace);
    const auto end = std::find_if_not(value.rbegin(), value.rend(), isSpace).base();
    if (begin >= end) return {};
    return std::string(begin, end);
}

void resetTransientTransactionFields(Transaction& transaction)
{
    transaction.valuta.clear();
}

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
        const auto parseResult = std::from_chars(rest.data(), rest.data() + rest.size(), idx);
        if (parseResult.ec == std::errc{} && parseResult.ptr == rest.data() + rest.size()) {
            maxIdx = std::max(maxIdx, idx);
        }
    }
    return maxIdx + 1;
}

}

namespace core::application {

std::string DraftFinalizer::finalize(AppState& state, const DraftStatement& draft)
{
    if (draft.transactions.empty()) return {};

    auto statement = std::make_shared<Statement>();
    statement->id = core::utils::makeStableId();
    statement->name = trimCopy(draft.name).empty()
        ? std::string(core::constants::appState::kDefaultImportedStatementName)
        : draft.name;
    state.statements.push_back(statement);

    for (const auto& item : draft.transactions) {
        auto transaction = std::make_shared<Transaction>();
        transaction->id = core::utils::makeStableId();
        transaction->name = item.name;
        transaction->bookingDate = item.bookingDate;
        transaction->amount = item.amount;
        transaction->description = item.description;
        transaction->statementId = statement->id;
        transaction->status = static_cast<Transaction::Status>(item.status);
        transaction->actorId = item.actorId;
        transaction->allocatable = item.allocatable;
        transaction->propertyIds = item.propertyIds;
        resetTransientTransactionFields(*transaction);

        const std::string normalizedType = trimCopy(item.type);
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

        state.transactions.push_back(transaction);
    }

    return statement->id;
}

}
