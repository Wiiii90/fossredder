#pragma once
#include "managers/IActorManager.h"
#include <vector>
#include <memory>
#include <string>

class Actor;

class ActorManager : public IActorManager {
public:
    void addActor(const std::string& name, const std::string& type = "", const std::string& description = "") override;
    std::vector<std::shared_ptr<Actor>> getActors() const override;
    void removeActor(const std::string& name) override;
    void updateActor(const std::string& name, const std::string& newName, const std::string& newType = "", const std::string& newDescription = "") override;
private:
    std::vector<std::shared_ptr<Actor>> actors_;
};