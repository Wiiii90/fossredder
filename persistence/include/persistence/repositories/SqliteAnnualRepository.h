#pragma once

#include "core/repositories/IAnnualRepository.h"
#include "persistence/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteAnnualRepository : public IAnnualRepository, public ISqlConnectionProvider {
public:
    explicit SqliteAnnualRepository(const std::string& dbPath);
    explicit SqliteAnnualRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteAnnualRepository() override;

    void addAnnual(const std::shared_ptr<Annual>& annual) override;
    std::vector<std::shared_ptr<Annual>> getAnnuals() const override;
    std::optional<std::shared_ptr<Annual>> getAnnualById(const std::string& id) const override;
    void removeAnnual(const std::string& id) override;
    void updateAnnual(const std::shared_ptr<Annual>& annual) override;

    void upsertAnnual(const std::shared_ptr<Annual>& annual) override;
    void clearAnnuals() override;

    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
