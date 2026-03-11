#pragma once

/**
 * @file core/include/core/repositories/IContractRepository.h
 * @brief Repository interface for Contract persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Contract;
}

class IContractRepository {
public:
    virtual ~IContractRepository() = default;

    /**
     * @brief Insert a new contract into the repository.
     * @param contract Shared pointer to the Contract to add.
     */
    virtual void addContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Return all contracts stored in the repository.
     * @return Vector of shared_ptr to Contract.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContracts() const = 0;

    /**
     * @brief Retrieve a contract by its identifier.
     * @param id Contract identifier string.
     * @return Optional shared_ptr to Contract if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Contract>> getContractById(const std::string& id) const = 0;

    /**
     * @brief Remove a contract identified by id from the repository.
     * @param id Contract identifier to remove.
     */
    virtual void removeContract(const std::string& id) = 0;

    /**
     * @brief Update an existing contract record in the repository.
     * @param contract Shared pointer to the Contract with updated fields.
     */
    virtual void updateContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Upsert a contract: insert or update depending on existence.
     * @param contract Shared pointer to the Contract to upsert.
     */
    virtual void upsertContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Remove all contracts from the repository.
     */
    virtual void clearContracts() = 0;

    /**
     * @brief Return contracts associated with a given actor id.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForActor(const std::string& actorId) const = 0;

    /**
     * @brief Return contracts associated with a given property id.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForProperty(const std::string& propertyId) const = 0;

    /**
     * @brief Return actor ids linked to a contract.
     */
    virtual std::vector<std::string> getActorIdsForContract(const std::string& contractId) const = 0;

    /**
     * @brief Return property ids linked to a contract.
     */
    virtual std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const = 0;
};
