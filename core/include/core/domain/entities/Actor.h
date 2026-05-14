/**
 * @file core/include/core/domain/entities/Actor.h
 * @brief Domain model for actors.
 */

#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include "core/domain/policies/AliasPolicy.h"
#include "core/domain/values/EntityName.h"
#include "core/domain/values/Alias.h"

namespace core::domain {

class Actor {
public:
    Actor();

    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(Actor&&) = delete;

    /**
     * @brief Renames the actor with normalized entity text.
     * @param value Normalized entity name wrapper.
     */
    void rename(EntityName value);
    /**
     * @brief Renames the actor with raw text.
     * @param value Raw entity name text.
     */
    void rename(std::string value);

    /**
     * @brief Adds a normalized alias when it is not already present.
     * @param value Alias to add.
     */
    void addAlias(Alias value);

    /**
     * @brief Removes an alias.
     * @param value Alias to remove.
     */
    void removeAlias(std::string value);

    /**
     * @brief Clears all aliases from the actor.
     */
    void clearAliases();

    /**
     * @brief Records a usage hit for an alias.
     * @param value Alias value.
     * @param timestamp Optional usage timestamp.
     */
    void recordAliasHit(std::string value, std::string timestamp = {});

    /**
     * @brief Checks whether an alias exists on the actor.
     * @param value Alias text to search for.
     * @return `true` when the alias exists.
     */
    [[nodiscard]] bool hasAlias(const std::string& value) const;

    /**
     * @brief Returns the number of aliases.
     * @return Alias count.
     */
    [[nodiscard]] std::size_t aliasCount() const noexcept;

private:
    std::string id_;
    std::string name_;
    std::vector<Alias> aliases_;
    std::string createdAt_;
    std::string updatedAt_;

public:
    // Read accessors
    [[nodiscard]] const std::string& id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::vector<Alias>& aliases() const noexcept;
    [[nodiscard]] const std::string& createdAt() const noexcept;
    [[nodiscard]] const std::string& updatedAt() const noexcept;

    // Technical hydration
    void setId(std::string value);
    void setAliases(std::vector<Alias> value);
    void setCreatedAt(std::string value);
    void setUpdatedAt(std::string value);

};

} // namespace core::domain
