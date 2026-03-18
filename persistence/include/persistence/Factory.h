/**
 * @file persistence/include/persistence/Factory.h
 * @brief Declares factories for SQLite storage dependencies and repository bundles.
 */

#pragma once

#include <memory>
#include <string>

#include "core/storage/RepositoryBundle.h"

namespace core::errors {
class IErrorReporter;
}

namespace core::storage {
class IRegistry;
}

class SqliteDb;

std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath);
std::shared_ptr<core::storage::IRegistry> createSqliteRegistry(const std::string& dbPath);

std::shared_ptr<class IActorRepository> createSqliteActorRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IPropertyRepository> createSqlitePropertyRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IPropertyRepository> createSqlitePropertyRepository(const std::shared_ptr<SqliteDb>& db,
                                                                          std::shared_ptr<core::errors::IErrorReporter> errorReporter);
std::shared_ptr<class IStatementRepository> createSqliteStatementRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IStatementRepository> createSqliteStatementRepository(const std::shared_ptr<SqliteDb>& db,
                                                                            std::shared_ptr<core::errors::IErrorReporter> errorReporter);
std::shared_ptr<class ITransactionRepository> createSqliteTransactionRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class ITransactionRepository> createSqliteTransactionRepository(const std::shared_ptr<SqliteDb>& db,
                                                                                std::shared_ptr<core::errors::IErrorReporter> errorReporter);
std::shared_ptr<class IContractRepository> createSqliteContractRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IAnalysisRepository> createSqliteAnalysisRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IAnnualRepository> createSqliteAnnualRepository(const std::shared_ptr<SqliteDb>& db);

core::storage::RepositoryBundle createSqliteRepositoryBundle(const std::shared_ptr<SqliteDb>& db);
core::storage::RepositoryBundle createSqliteRepositoryBundle(const std::shared_ptr<SqliteDb>& db,
                                                             std::shared_ptr<core::errors::IErrorReporter> errorReporter);
