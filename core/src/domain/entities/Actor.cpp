/**
 * @file core/src/domain/entities/Actor.cpp
 * @brief Domain behavior for actors.
 */

#include "core/domain/entities/Actor.h"

#include <algorithm>
#include <utility>

namespace core::domain {

Actor::Actor() = default;

void Actor::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Actor::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Actor::setAliases(std::vector<Alias> value) {
    policies::alias::normalizeAliases(value);
    aliases_ = std::move(value);
}

void Actor::addAlias(Alias value) {
    policies::alias::normalizeAlias(value);
    if (value.value().empty() || policies::alias::containsAliasValue(aliases_, value.value())) {
        return;
    }
    aliases_.push_back(std::move(value));
}

void Actor::removeAlias(std::string value) {
    const auto normalized = policies::alias::canonicalAliasValue(value);
    policies::alias::removeAliasValue(aliases_, normalized);
}

void Actor::clearAliases() {
    aliases_.clear();
}

void Actor::recordAliasHit(std::string value, std::string timestamp) {
    policies::alias::recordAliasHit(aliases_, std::move(value), std::move(timestamp));
}

bool Actor::hasAlias(const std::string& value) const {
    return policies::alias::containsAliasValue(aliases_, value);
}

std::size_t Actor::aliasCount() const noexcept {
    return aliases_.size();
}

const std::string& Actor::id() const noexcept { return id_; }
const std::string& Actor::name() const noexcept { return name_; }
const std::vector<Alias>& Actor::aliases() const noexcept { return aliases_; }
const std::string& Actor::createdAt() const noexcept { return createdAt_; }
const std::string& Actor::updatedAt() const noexcept { return updatedAt_; }
void Actor::setId(std::string value) { id_ = std::move(value); }
void Actor::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Actor::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
