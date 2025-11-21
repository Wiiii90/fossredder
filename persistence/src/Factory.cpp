#include "persistence/Factory.h"
#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/repositories/SqliteConfigRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"
#include "persistence/repositories/SqliteBookingGroupRepository.h"

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

std::shared_ptr<IBookingGroupRepository> createSqliteBookingGroupRepository(const std::string& dbPath) {
    return std::make_shared<SqliteBookingGroupRepository>(dbPath);
}
