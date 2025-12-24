#include "core/pch.h"
#include "core/models/Contract.h"

#include <algorithm>

#include "core/models/Actor.h"
#include "core/models/Property.h"

void Contract::addActor(Actor* actor) {
    if (!actor) return;
    if (std::find(actors.begin(), actors.end(), actor) != actors.end()) return;
    actors.push_back(actor);
    if (!actor->id.empty() && std::find(actorIds.begin(), actorIds.end(), actor->id) == actorIds.end()) actorIds.push_back(actor->id);
}

bool Contract::removeActor(Actor* actor) {
    auto it = std::find(actors.begin(), actors.end(), actor);
    if (it == actors.end()) return false;
    if (actor && !actor->id.empty()) {
        auto idIt = std::find(actorIds.begin(), actorIds.end(), actor->id);
        if (idIt != actorIds.end()) actorIds.erase(idIt);
    }
    actors.erase(it);
    return true;
}

void Contract::addProperty(Property* property) {
    if (!property) return;
    if (std::find(properties.begin(), properties.end(), property) != properties.end()) return;
    properties.push_back(property);
    if (!property->id.empty() && std::find(propertyIds.begin(), propertyIds.end(), property->id) == propertyIds.end()) propertyIds.push_back(property->id);
}

bool Contract::removeProperty(Property* property) {
    auto it = std::find(properties.begin(), properties.end(), property);
    if (it == properties.end()) return false;
    if (property && !property->id.empty()) {
        auto idIt = std::find(propertyIds.begin(), propertyIds.end(), property->id);
        if (idIt != propertyIds.end()) propertyIds.erase(idIt);
    }
    properties.erase(it);
    return true;
}
