#include "persistence/Factory.h"
#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/repositories/SqliteConfigRepository.h"
#include "persistence/repositories/SqliteTransactionRepository.h"
#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/SqliteDb.h"
#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/repositories/SqliteAnnualRepository.h"

// Helper that creates a SqliteDb instance from a path. Primary factory
// functions prefer the shared_ptr<SqliteDb> overload for reuse and DI.
std::shared_ptr<SqliteDb> createSqliteDb(const std::string& dbPath) {
    return std::make_shared<SqliteDb>(dbPath);
}

// No dbPath overloads implemented here. Use createSqliteDb() then pass the
// returned shared_ptr to the primary factory functions.

// Primary constructors taking an existing shared SqliteDb
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

std::shared_ptr<IAnalysisRepository> createSqliteAnalysisRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteAnalysisRepository>(db);
}

std::shared_ptr<IAnnualRepository> createSqliteAnnualRepository(const std::shared_ptr<SqliteDb>& db) {
    return std::make_shared<SqliteAnnualRepository>(db);
}
