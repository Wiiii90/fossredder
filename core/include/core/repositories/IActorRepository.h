#pragma once

#include <memory>
#include <string>
#include <vector>

class Actor;

class IActorRepository {
public:
    virtual ~IActorRepository() = default;
    virtual void addActor(const std::string& name, const std::string& type = "", const std::string& description = "") = 0;
    virtual std::vector<std::shared_ptr<Actor>> getActors() const = 0;
    virtual void removeActor(const std::string& name) = 0;
    virtual void updateActor(const std::string& name, const std::string& newName, const std::string& newType = "", const std::string& newDescription = "") = 0;
};
