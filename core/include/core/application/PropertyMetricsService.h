/**
 * @file core/include/core/application/PropertyMetricsService.h
 * @brief Declares property-level transaction metric aggregation helpers.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core::domain {
class Contract;
class Transaction;
}

namespace core::application {

struct PropertySums {
    double total = 0.0;
    double allocatable = 0.0;
    double nonAllocatable = 0.0;
};

class PropertyMetricsService {
public:
    static std::vector<std::string> propertyContractTypes(
        const std::string& propertyId,
        const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions,
        const std::vector<std::shared_ptr<core::domain::Contract>>& contracts);

    static PropertySums propertySums(
        const std::string& propertyId,
        const std::string& contractType,
        const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions,
        const std::vector<std::shared_ptr<core::domain::Contract>>& contracts);
};

} // namespace core::application
