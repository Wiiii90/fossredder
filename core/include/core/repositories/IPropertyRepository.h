#pragma once

/**
 * @file core/include/core/repositories/IPropertyRepository.h
 * @brief Repository interface for Property persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Property;

class IPropertyRepository {
public:
    virtual ~IPropertyRepository() = default;

    /**
     * @brief Insert a new property into the repository.
     * @param property Shared pointer to the Property to add.
     */
    virtual void addProperty(const std::shared_ptr<Property>& property) = 0;

    /**
     * @brief Return all properties stored in the repository.
     * @return Vector of shared_ptr to Property.
     */
    virtual std::vector<std::shared_ptr<Property>> getProperties() const = 0;

    /**
     * @brief Retrieve a property by its identifier.
     * @param id Property identifier string.
     * @return Optional shared_ptr to Property if found.
     */
    virtual std::optional<std::shared_ptr<Property>> getPropertyById(const std::string& id) const = 0;

    /**
     * @brief Remove a property identified by id from the repository.
     * @param id Property identifier to remove.
     */
    virtual void removeProperty(const std::string& id) = 0;

    /**
     * @brief Update an existing property record in the repository.
     * @param property Shared pointer to the Property with updated fields.
     */
    virtual void updateProperty(const std::shared_ptr<Property>& property) = 0;

    /**
     * @brief Upsert a property: insert or update depending on existence.
     * @param property Shared pointer to the Property to upsert.
     */
    virtual void upsertProperty(const std::shared_ptr<Property>& property) = 0;

    /**
     * @brief Remove all properties from the repository.
     */
    virtual void clearProperties() = 0;
};
