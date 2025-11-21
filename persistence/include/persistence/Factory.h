#pragma once

#include <memory>
#include <string>

std::shared_ptr<class IActorRepository> createSqliteActorRepository(const std::string& dbPath);
std::shared_ptr<class IPropertyRepository> createSqlitePropertyRepository(const std::string& dbPath);
std::shared_ptr<class IStatementRepository> createSqliteStatementRepository(const std::string& dbPath);
std::shared_ptr<class IConfigRepository> createSqliteConfigRepository(const std::string& dbPath);
std::shared_ptr<class ITransactionRepository> createSqliteTransactionRepository(const std::string& dbPath);
std::shared_ptr<class IBookingGroupRepository> createSqliteBookingGroupRepository(const std::string& dbPath);
