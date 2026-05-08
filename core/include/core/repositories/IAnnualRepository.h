/**
 * @file core/include/core/repositories/IAnnualRepository.h
 * @brief Repository interface for core::domain::Annual persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Annual;
}

class IAnnualRepository {
public:
    virtual ~IAnnualRepository() = default;

    /**
     * @brief Insert a new annual into the repository.
     * @param annual Shared pointer to the core::domain::Annual to add.
     */
    virtual void addAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Retrieve all stored annuals.
     * @return All annuals stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::Annual>> getAnnuals() const = 0;

    /**
     * @brief Retrieve an annual by its identifier.
     * @param id core::domain::Annual identifier string.
     * @return The annual with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Annual>> getAnnualById(const std::string& id) const = 0;

    /**
     * @brief Remove an annual identified by id from the repository.
     * @param id core::domain::Annual identifier to remove.
     */
    virtual void removeAnnual(const std::string& id) = 0;

    /**
     * @brief Update an existing annual record in the repository.
     * @param annual Shared pointer to the core::domain::Annual with updated fields.
     */
    virtual void updateAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Upsert an annual: insert or update depending on existence.
     * @param annual Shared pointer to the core::domain::Annual to upsert.
     */
    virtual void upsertAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;

    /**
     * @brief Remove all annuals from the repository.
     */
    virtual void clearAnnuals() = 0;
};
