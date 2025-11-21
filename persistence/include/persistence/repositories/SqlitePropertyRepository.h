#pragma once

#include "core/repositories/IPropertyRepository.h"
#include <memory>
#include <string>

class SqlitePropertyRepository : public IPropertyRepository {
public:
    explicit SqlitePropertyRepository(const std::string& dbPath);
    ~SqlitePropertyRepository() override;

    void addProperty(const std::string& name, const std::string& address, const std::string& description) override;
    std::vector<std::shared_ptr<Property>> getProperties() const override;
    void removeProperty(const std::string& name) override;
    void updateProperty(const std::string& name, const std::string& address, const std::string& description) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
