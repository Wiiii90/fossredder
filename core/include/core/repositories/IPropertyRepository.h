/**
 * @file core/include/core/repositories/IPropertyRepository.h
 * @brief Repository interface for core::domain::Property persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Property;
}

class IPropertyRepository {
public:
    virtual ~IPropertyRepository() = default;

    /**
     * @brief Insert a new property into the repository.
     * @param property Shared pointer to the core::domain::Property to add.
     */
    virtual void addProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Retrieve all stored properties.
     * @return All properties stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::Property>> getProperties() const = 0;

    /**
     * @brief Retrieve a property by its identifier.
     * @param id Core property identifier.
     * @return The property with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Property>> getPropertyById(const std::string& id) const = 0;

    /**
     * @brief Remove a property identified by id from the repository.
     * @param id core::domain::Property identifier to remove.
     */
    virtual void removeProperty(const std::string& id) = 0;

    /**
     * @brief Update an existing property record in the repository.
     * @param property Shared pointer to the core::domain::Property with updated fields.
     */
    virtual void updateProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Upsert a property: insert or update depending on existence.
     * @param property Shared pointer to the core::domain::Property to upsert.
     */
    virtual void upsertProperty(const std::shared_ptr<core::domain::Property>& property) = 0;

    /**
     * @brief Remove all properties from the repository.
     */
    virtual void clearProperties() = 0;
};
