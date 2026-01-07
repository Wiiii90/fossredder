#pragma once

/**
 * @file core/include/core/repositories/IContractRepository.h
 * @brief Repository interface for Contract persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Contract;

class IContractRepository {
public:
    virtual ~IContractRepository() = default;

    /**
     * @brief Insert a new contract into the repository.
     * @param contract Shared pointer to the Contract to add.
     */
    virtual void addContract(const std::shared_ptr<Contract>& contract) = 0;

    /**
     * @brief Return all contracts stored in the repository.
     * @return Vector of shared_ptr to Contract.
     */
    virtual std::vector<std::shared_ptr<Contract>> getContracts() const = 0;

    /**
     * @brief Retrieve a contract by its identifier.
     * @param id Contract identifier string.
     * @return Optional shared_ptr to Contract if found.
     */
    virtual std::optional<std::shared_ptr<Contract>> getContractById(const std::string& id) const = 0;

    /**
     * @brief Remove a contract identified by id from the repository.
     * @param id Contract identifier to remove.
     */
    virtual void removeContract(const std::string& id) = 0;

    /**
     * @brief Update an existing contract record in the repository.
     * @param contract Shared pointer to the Contract with updated fields.
     */
    virtual void updateContract(const std::shared_ptr<Contract>& contract) = 0;

    /**
     * @brief Upsert a contract: insert or update depending on existence.
     * @param contract Shared pointer to the Contract to upsert.
     */
    virtual void upsertContract(const std::shared_ptr<Contract>& contract) = 0;

    /**
     * @brief Remove all contracts from the repository.
     */
    virtual void clearContracts() = 0;
};
