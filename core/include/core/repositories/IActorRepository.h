#pragma once

#include <memory>
#include <string>
#include <vector>

class Actor;

class IActorRepository {
public:
    virtual ~IActorRepository() = default;

    virtual void addActor(const std::shared_ptr<Actor>& actor) = 0;
    virtual std::vector<std::shared_ptr<Actor>> getActors() const = 0;
    virtual std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const = 0;
    virtual void removeActor(const std::string& id) = 0;
    virtual void updateActor(const std::shared_ptr<Actor>& actor) = 0;
};
