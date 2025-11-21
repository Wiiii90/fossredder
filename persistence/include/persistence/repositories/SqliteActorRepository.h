#pragma once

#include "core/repositories/IActorRepository.h"
#include <memory>
#include <string>

class SqliteActorRepository : public IActorRepository {
public:
    explicit SqliteActorRepository(const std::string& dbPath);
    ~SqliteActorRepository() override;

    void addActor(const std::string& name, const std::string& type = "", const std::string& description = "") override;
    std::vector<std::shared_ptr<Actor>> getActors() const override;
    void removeActor(const std::string& name) override;
    void updateActor(const std::string& name, const std::string& newName, const std::string& newType = "", const std::string& newDescription = "") override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_; // holds sqlite connection and prepared statements
};
