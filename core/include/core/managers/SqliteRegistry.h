#pragma once

#include "core/managers/IRegistry.h"
#include <memory>
#include <string>

struct sqlite3;
class SqliteDb;

class SqliteRegistry : public IRegistry {
public:
    explicit SqliteRegistry(const std::string& dbPath);
    ~SqliteRegistry();

    std::optional<std::string> getLatest() const override;
    void setLatest(const std::string& path) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
