#pragma once

/**
 * @file core/include/core/repositories/IActorRepository.h
 * @brief Repository interface for Actor persistence operations.
 *
 * Abstract interface used by higher level components to persist and query
 * Actor entities. Implementations provide concrete storage (e.g. sqlite).
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Actor;

class IActorRepository {
public:
    virtual ~IActorRepository() = default;

    /**
     * @brief Insert a new actor into the repository.
     * @param actor Shared pointer to the Actor to add.
     */
    virtual void addActor(const std::shared_ptr<Actor>& actor) = 0;

    /**
     * @brief Return all actors stored in the repository.
     * @return Vector of shared_ptr to Actor.
     */
    virtual std::vector<std::shared_ptr<Actor>> getActors() const = 0;

    /**
     * @brief Retrieve an actor by its identifier.
     * @param id Actor identifier string.
     * @return Optional shared_ptr to Actor if found.
     */
    virtual std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const = 0;

    /**
     * @brief Remove an actor identified by id from the repository.
     * @param id Actor identifier to remove.
     */
    virtual void removeActor(const std::string& id) = 0;

    /**
     * @brief Update an existing actor record in the repository.
     * @param actor Shared pointer to the Actor with updated fields.
     */
    virtual void updateActor(const std::shared_ptr<Actor>& actor) = 0;

    /**
     * @brief Upsert an actor: insert or update depending on existence.
     * @param actor Shared pointer to the Actor to upsert.
     */
    virtual void upsertActor(const std::shared_ptr<Actor>& actor) = 0;

    /**
     * @brief Remove all actors from the repository.
     */
    virtual void clearActors() = 0;
};
