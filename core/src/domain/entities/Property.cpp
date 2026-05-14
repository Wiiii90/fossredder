/**
 * @file core/src/domain/entities/Property.cpp
 * @brief Domain behavior for properties.
 */

#include "core/domain/entities/Property.h"

#include <algorithm>
#include <utility>

namespace core::domain {

Property::Property() = default;

void Property::rename(EntityName value) {
    name_ = std::move(value.value());
}

void Property::rename(std::string value) {
    rename(EntityName(std::move(value)));
}

void Property::setAliases(std::vector<Alias> value) {
    policies::alias::normalizeAliases(value);
    aliases_ = std::move(value);
}

void Property::addAlias(Alias value) {
    policies::alias::normalizeAlias(value);
    if (value.value().empty() || policies::alias::containsAliasValue(aliases_, value.value())) {
        return;
    }
    aliases_.push_back(std::move(value));
}

void Property::removeAlias(std::string value) {
    const auto normalized = policies::alias::canonicalAliasValue(value);
    policies::alias::removeAliasValue(aliases_, normalized);
}

void Property::clearAliases() {
    aliases_.clear();
}

void Property::recordAliasHit(std::string value, std::string timestamp) {
    policies::alias::recordAliasHit(aliases_, std::move(value), std::move(timestamp));
}

bool Property::hasAlias(const std::string& value) const {
    return policies::alias::containsAliasValue(aliases_, value);
}

std::size_t Property::aliasCount() const noexcept {
    return aliases_.size();
}

const std::string& Property::id() const noexcept { return id_; }
const std::string& Property::name() const noexcept { return name_; }
const std::vector<Alias>& Property::aliases() const noexcept { return aliases_; }
const std::string& Property::createdAt() const noexcept { return createdAt_; }
const std::string& Property::updatedAt() const noexcept { return updatedAt_; }
void Property::setId(std::string value) { id_ = std::move(value); }
void Property::setCreatedAt(std::string value) { createdAt_ = std::move(value); }
void Property::setUpdatedAt(std::string value) { updatedAt_ = std::move(value); }

} // namespace core::domain
