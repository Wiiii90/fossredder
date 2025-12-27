#include "persistence/Factory.h"
#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/repositories/SqliteConfigRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"
#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/SqliteDb.h"

std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath) {
    return std::make_shared<SqliteDb>(dbPath);
}

std::shared_ptr<IActorRepository> createSqliteActorRepository(const std::string& dbPath) {
    return std::make_shared<SqliteActorRepository>(dbPath);
}

std::shared_ptr<IPropertyRepository> createSqlitePropertyRepository(const std::string& dbPath) {
    return std::make_shared<SqlitePropertyRepository>(dbPath);
}

std::shared_ptr<IStatementRepository> createSqliteStatementRepository(const std::string& dbPath) {
    return std::make_shared<SqliteStatementRepository>(dbPath);
}

std::shared_ptr<IConfigRepository> createSqliteConfigRepository(const std::string& dbPath) {
    return std::make_shared<SqliteConfigRepository>(dbPath);
}

std::shared_ptr<ITransactionRepository> createSqliteTransactionRepository(const std::string& dbPath) {
    return std::make_shared<SqliteTransactionRepository>(dbPath);
}

std::shared_ptr<IContractRepository> createSqliteContractRepository(const std::string& dbPath) {
    return std::make_shared<SqliteContractRepository>(dbPath);
}

std::shared_ptr<IActorRepository> createSqliteActorRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteActorRepository>(db);
}

std::shared_ptr<IPropertyRepository> createSqlitePropertyRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqlitePropertyRepository>(db);
}

std::shared_ptr<IStatementRepository> createSqliteStatementRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteStatementRepository>(db);
}

std::shared_ptr<IConfigRepository> createSqliteConfigRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteConfigRepository>(db);
}

std::shared_ptr<ITransactionRepository> createSqliteTransactionRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteTransactionRepository>(db);
}

std::shared_ptr<IContractRepository> createSqliteContractRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteContractRepository>(db);
}