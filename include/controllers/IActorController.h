#pragma once
#include <memory>
#include <string>
#include <vector>

class Actor;

class IActorController {
public:
    virtual ~IActorController() = default;
    virtual void addActor(const std::string& name) = 0;
    virtual std::vector<std::shared_ptr<Actor>> getActors() const = 0;
    virtual void removeActor(const std::string& name) = 0;
    virtual void updateActor(const std::string& name, const std::string& newName) = 0;
};