#pragma once

#include "core/repositories/IActorRepository.h"
#include "core/repositories/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteActorRepository : public IActorRepository, public ISqlConnectionProvider {
public:
    explicit SqliteActorRepository(const std::string& dbPath);
    explicit SqliteActorRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteActorRepository() override;

    void addActor(const std::shared_ptr<Actor>& actor) override;
    std::vector<std::shared_ptr<Actor>> getActors() const override;
    std::optional<std::shared_ptr<Actor>> getActorById(const std::string& id) const override;
    void removeActor(const std::string& id) override;
    void updateActor(const std::shared_ptr<Actor>& actor) override;

    void upsertActor(const std::shared_ptr<Actor>& actor) override;
    void clearActors() override;

    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
