#pragma once

#include "core/repositories/IConfigRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteConfigRepository : public IConfigRepository {
public:
    explicit SqliteConfigRepository(const std::string& dbPath);
    explicit SqliteConfigRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteConfigRepository() override;

    bool saveConfig(const std::string& name, const std::shared_ptr<core::domain::Config>& config) override;
    std::optional<std::shared_ptr<core::domain::Config>> loadConfig(const std::string& name) const override;
    std::vector<std::string> listConfigs() const override;
    bool deleteConfig(const std::string& name) override;
    void setDefaultConfig(const std::string& name) override;
    std::optional<std::shared_ptr<core::domain::Config>> getDefaultConfig() const override;
    std::string getDefaultConfigName() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
