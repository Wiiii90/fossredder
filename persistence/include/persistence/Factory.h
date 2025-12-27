#pragma once

#include <memory>
#include <string>

class SqliteDb;

std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath);

std::shared_ptr<class IActorRepository> createSqliteActorRepository(const std::string& dbPath);
std::shared_ptr<class IPropertyRepository> createSqlitePropertyRepository(const std::string& dbPath);
std::shared_ptr<class IStatementRepository> createSqliteStatementRepository(const std::string& dbPath);
std::shared_ptr<class IConfigRepository> createSqliteConfigRepository(const std::string& dbPath);
std::shared_ptr<class ITransactionRepository> createSqliteTransactionRepository(const std::string& dbPath);
std::shared_ptr<class IContractRepository> createSqliteContractRepository(const std::string& dbPath);

std::shared_ptr<class IActorRepository> createSqliteActorRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IPropertyRepository> createSqlitePropertyRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IStatementRepository> createSqliteStatementRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IConfigRepository> createSqliteConfigRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class ITransactionRepository> createSqliteTransactionRepository(const std::shared_ptr<SqliteDb>& db);
std::shared_ptr<class IContractRepository> createSqliteContractRepository(const std::shared_ptr<SqliteDb>& db);
