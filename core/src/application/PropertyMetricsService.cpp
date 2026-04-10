/**
 * @file core/src/application/PropertyMetricsService.cpp
 * @brief Implements property-level transaction metric aggregation helpers.
 */

#include "core/application/PropertyMetricsService.h"

#include <algorithm>
#include <unordered_map>

#include "core/models/Contract.h"
#include "core/models/Transaction.h"

namespace core::application {

namespace {

std::unordered_map<std::string, std::string> buildContractTypeById(
    const std::vector<std::shared_ptr<core::domain::Contract>>& contracts)
{
    std::unordered_map<std::string, std::string> out;
    out.reserve(contracts.size());
    for (const auto& contract : contracts) {
        if (!contract || contract->id.empty()) continue;
        out.emplace(contract->id, contract->type);
    }
    return out;
}

bool containsPropertyId(const std::vector<std::string>& propertyIds, const std::string& propertyId)
{
    return std::find(propertyIds.begin(), propertyIds.end(), propertyId) != propertyIds.end();
}

bool containsValue(const std::vector<std::string>& values, const std::string& value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

std::vector<std::string> PropertyMetricsService::propertyContractTypes(
    const std::string& propertyId,
    const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions,
    const std::vector<std::shared_ptr<core::domain::Contract>>& contracts)
{
    std::vector<std::string> out;
    if (propertyId.empty()) return out;

    const auto contractTypeById = buildContractTypeById(contracts);
    for (const auto& transaction : transactions) {
        if (!transaction) continue;
        if (!containsPropertyId(transaction->propertyIds, propertyId)) continue;
        if (transaction->contractId.empty()) continue;

        const auto it = contractTypeById.find(transaction->contractId);
        if (it == contractTypeById.end()) continue;
        if (it->second.empty()) continue;
        if (!containsValue(out, it->second)) out.push_back(it->second);
    }

    return out;
}

PropertySums PropertyMetricsService::propertySums(
    const std::string& propertyId,
    const std::string& contractType,
    const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions,
    const std::vector<std::shared_ptr<core::domain::Contract>>& contracts)
{
    PropertySums sums;
    if (propertyId.empty()) return sums;

    const auto contractTypeById = buildContractTypeById(contracts);
    for (const auto& transaction : transactions) {
        if (!transaction) continue;
        if (!containsPropertyId(transaction->propertyIds, propertyId)) continue;

        if (!contractType.empty()) {
            std::string currentType;
            if (!transaction->contractId.empty()) {
                const auto it = contractTypeById.find(transaction->contractId);
                if (it != contractTypeById.end()) currentType = it->second;
            }
            if (currentType != contractType) continue;
        }

        sums.total += transaction->amount;
        if (transaction->allocatable) sums.allocatable += transaction->amount;
        else sums.nonAllocatable += transaction->amount;
    }

    return sums;
}

} // namespace core::application
