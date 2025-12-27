#pragma once

#include "core/repositories/IPropertyRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqlitePropertyRepository : public IPropertyRepository {
public:
    explicit SqlitePropertyRepository(const std::string& dbPath);
    explicit SqlitePropertyRepository(std::shared_ptr<SqliteDb> db);
    ~SqlitePropertyRepository() override;

    void addProperty(const std::shared_ptr<Property>& property) override;
    std::vector<std::shared_ptr<Property>> getProperties() const override;
    std::optional<std::shared_ptr<Property>> getPropertyById(const std::string& id) const override;
    void removeProperty(const std::string& id) override;
    void updateProperty(const std::shared_ptr<Property>& property) override;

    void upsertProperty(const std::shared_ptr<Property>& property) override;
    void clearProperties() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
