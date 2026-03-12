/**
 * @file persistence/include/persistence/repositories/SqliteStatementRepository.h
 * @brief Declares the SQLite-backed statement repository.
 */

#pragma once

#include "core/errors/IErrorReporter.h"
#include "core/repositories/IStatementRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteStatementRepository : public IStatementRepository {
public:
    explicit SqliteStatementRepository(const std::string& dbPath);
    SqliteStatementRepository(const std::string& dbPath, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    explicit SqliteStatementRepository(std::shared_ptr<SqliteDb> db);
    SqliteStatementRepository(std::shared_ptr<SqliteDb> db, std::shared_ptr<core::errors::IErrorReporter> errorReporter);
    ~SqliteStatementRepository() override;

    void addStatement(const std::shared_ptr<core::domain::Statement>& statement) override;
    std::vector<std::shared_ptr<core::domain::Statement>> getStatements() const override;
    std::optional<std::shared_ptr<core::domain::Statement>> getStatementById(const std::string& id) const override;
    void removeStatement(const std::string& id) override;
    void updateStatement(const std::shared_ptr<core::domain::Statement>& statement) override;

    void upsertStatement(const std::shared_ptr<core::domain::Statement>& statement) override;
    void clearStatements() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
