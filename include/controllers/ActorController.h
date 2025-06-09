#pragma once
#include "controllers/IActorController.h"
#include <memory>
#include <vector>
#include <string>

class Actor;

class ActorController : public IActorController {
public:
    void addActor(const std::string& name) override;
    std::vector<std::shared_ptr<Actor>> getActors() const override;
    void removeActor(const std::string& name) override;
    void updateActor(const std::string& name, const std::string& newName) override;
private:
    std::vector<std::shared_ptr<Actor>> actors;
};