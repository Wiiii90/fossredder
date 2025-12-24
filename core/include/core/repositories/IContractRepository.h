#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Contract;

class IContractRepository {
public:
    virtual ~IContractRepository() = default;

    virtual void addContract(const std::shared_ptr<Contract>& contract) = 0;
    virtual std::vector<std::shared_ptr<Contract>> getContracts() const = 0;
    virtual std::optional<std::shared_ptr<Contract>> getContractById(const std::string& id) const = 0;
    virtual void removeContract(const std::string& id) = 0;
    virtual void updateContract(const std::shared_ptr<Contract>& contract) = 0;
};
