#pragma once

/**
 * @file core/include/core/repositories/IActorRepository.h
 * @brief Repository interface for core::domain::Actor persistence operations.
 *
 * Abstract interface used by higher level components to persist and query
 * core::domain::Actor entities. Implementations provide concrete storage (e.g. sqlite).
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class core::domain::Actor;
}

class IActorRepository {
public:
    virtual ~IActorRepository() = default;

    /**
     * @brief Insert a new actor into the repository.
     * @param actor Shared pointer to the core::domain::Actor to add.
     */
    virtual void addActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Return all actors stored in the repository.
     * @return Vector of shared_ptr to core::domain::Actor.
     */
    virtual std::vector<std::shared_ptr<core::domain::Actor>> getActors() const = 0;

    /**
     * @brief Retrieve an actor by its identifier.
     * @param id core::domain::Actor identifier string.
     * @return Optional shared_ptr to core::domain::Actor if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Actor>> getActorById(const std::string& id) const = 0;

    /**
     * @brief Remove an actor identified by id from the repository.
     * @param id core::domain::Actor identifier to remove.
     */
    virtual void removeActor(const std::string& id) = 0;

    /**
     * @brief Update an existing actor record in the repository.
     * @param actor Shared pointer to the core::domain::Actor with updated fields.
     */
    virtual void updateActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Upsert an actor: insert or update depending on existence.
     * @param actor Shared pointer to the core::domain::Actor to upsert.
     */
    virtual void upsertActor(const std::shared_ptr<core::domain::Actor>& actor) = 0;

    /**
     * @brief Remove all actors from the repository.
     */
    virtual void clearActors() = 0;
};
