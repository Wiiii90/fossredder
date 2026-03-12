/**
 * @file persistence/include/persistence/repositories/SqlitePropertyRepository.h
 * @brief Declares the SQLite-backed property repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/repositories/IPropertyRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqlitePropertyRepository : public IPropertyRepository {
public:
    explicit SqlitePropertyRepository(const std::string& dbPath);
    SqlitePropertyRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    explicit SqlitePropertyRepository(std::shared_ptr<SqliteDb> db);
    SqlitePropertyRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    ~SqlitePropertyRepository() override;

    void addProperty(const std::shared_ptr<core::domain::Property>& property) override;
    std::vector<std::shared_ptr<core::domain::Property>> getProperties() const override;
    std::optional<std::shared_ptr<core::domain::Property>> getPropertyById(const std::string& id) const override;
    void removeProperty(const std::string& id) override;
    void updateProperty(const std::shared_ptr<core::domain::Property>& property) override;

    void upsertProperty(const std::shared_ptr<core::domain::Property>& property) override;
    void clearProperties() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
