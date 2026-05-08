/**
 * @file core/include/core/repositories/IContractRepository.h
 * @brief Repository interface for core::domain::Contract persistence operations.
 */

#pragma once

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
     * @param contract Shared pointer to the core::domain::Contract to add.
     */
    virtual void addContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Retrieve all stored contracts.
     * @return All contracts stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContracts() const = 0;

    /**
     * @brief Retrieve a contract by its identifier.
     * @param id core::domain::Contract identifier string.
     * @return The contract with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Contract>> getContractById(const std::string& id) const = 0;

    /**
     * @brief Remove a contract identified by id from the repository.
     * @param id core::domain::Contract identifier to remove.
     */
    virtual void removeContract(const std::string& id) = 0;

    /**
     * @brief Update an existing contract record in the repository.
     * @param contract Shared pointer to the core::domain::Contract with updated fields.
     */
    virtual void updateContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Upsert a contract: insert or update depending on existence.
     * @param contract Shared pointer to the core::domain::Contract to upsert.
     */
    virtual void upsertContract(const std::shared_ptr<core::domain::Contract>& contract) = 0;

    /**
     * @brief Remove all contracts from the repository.
     */
    virtual void clearContracts() = 0;

    /**
     * @brief Retrieve contracts associated with a given actor id.
     * @param actorId Actor identifier.
     * @return Contracts linked to the given actor id.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForActor(const std::string& actorId) const = 0;

    /**
     * @brief Retrieve contracts associated with a given property id.
     * @param propertyId Property identifier.
     * @return Contracts linked to the given property id.
     */
    virtual std::vector<std::shared_ptr<core::domain::Contract>> getContractsForProperty(const std::string& propertyId) const = 0;

    /**
     * @brief Retrieve actor ids linked to a contract.
     * @param contractId Contract identifier.
     * @return Actor identifiers linked to the contract.
     */
    virtual std::vector<std::string> getActorIdsForContract(const std::string& contractId) const = 0;

    /**
     * @brief Retrieve property ids linked to a contract.
     * @param contractId Contract identifier.
     * @return Property identifiers linked to the contract.
     */
    virtual std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const = 0;
};
