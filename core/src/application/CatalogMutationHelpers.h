/**
 * @file core/src/application/CatalogMutationHelpers.h
 * @brief Declares private generic mutation helpers used by `CatalogService`.
 */

#pragma once

#include "core/utils/StableId.h"

#include <algorithm>
#include <memory>
#include <string>

namespace core::application::detail {

template <typename Collection, typename Configure>
std::string appendEntity(Collection& collection, Configure&& configure)
{
    using Entity = typename Collection::value_type::element_type;
    auto entity = std::make_shared<Entity>();
    entity->id = core::utils::makeStableId();
    configure(*entity);
    const auto id = entity->id;
    collection.push_back(std::move(entity));
    return id;
}

template <typename Collection, typename Update>
bool updateEntity(Collection& collection, const std::string& id, Update&& update)
{
    if (id.empty()) return false;
    for (auto& entity : collection) {
        if (!entity || entity->id != id) continue;
        update(*entity);
        return true;
    }
    return false;
}

template <typename Collection>
bool eraseEntity(Collection& collection, const std::string& id)
{
    if (id.empty()) return false;
    const auto originalSize = collection.size();
    collection.erase(std::remove_if(collection.begin(), collection.end(), [&](const auto& entity) {
        return entity && entity->id == id;
    }), collection.end());
    return collection.size() != originalSize;
}

} // namespace core::application::detail
