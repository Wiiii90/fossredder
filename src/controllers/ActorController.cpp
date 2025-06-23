#include "controllers/ActorController.h"
#include "models/Actor.h"
#include <algorithm>

void ActorController::addActor(const std::string& name, const std::string& type, const std::string& description) {
    actors_.push_back(std::make_shared<Actor>(name, type, description));
}

std::vector<std::shared_ptr<Actor>> ActorController::getActors() const {
    return actors_;
}

void ActorController::removeActor(const std::string& name) {
    actors_.erase(
        std::remove_if(actors_.begin(), actors_.end(),
            [&](const std::shared_ptr<Actor>& a) { return a && a->name == name; }),
        actors_.end());
}

void ActorController::updateActor(const std::string& name, const std::string& newName, const std::string& newType, const std::string& newDescription) {
    for (auto& a : actors_) {
        if (a && a->name == name) {
            a->name = newName;
            a->type = newType;
            a->description = newDescription;
            break;
        }
    }
}