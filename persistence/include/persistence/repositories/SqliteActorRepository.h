#pragma once

#include "core/repositories/IActorRepository.h"
#include <memory>
#include <string>

class SqliteActorRepository : public IActorRepository {
public:
    explicit SqliteActorRepository(const std::string& dbPath);
    ~SqliteActorRepository() override;

    void addActor(const std::shared_ptr<Actor>& actor) override;
    std::vector<std::shared_ptr<Actor>> getActors() const override;
    std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const override;
    void removeActor(const std::string& id) override;
    void updateActor(const std::shared_ptr<Actor>& actor) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
