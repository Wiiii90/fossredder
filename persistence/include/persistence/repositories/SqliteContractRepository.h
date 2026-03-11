#pragma once

#include "core/repositories/IContractRepository.h"
#include <memory>
#include <string>

class SqliteDb;

class SqliteContractRepository : public IContractRepository {
public:
    explicit SqliteContractRepository(const std::string& dbPath);
    explicit SqliteContractRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteContractRepository() override;

    void addContract(const std::shared_ptr<core::domain::Contract>& contract) override;
    std::vector<std::shared_ptr<core::domain::Contract>> getContracts() const override;
    std::optional<std::shared_ptr<core::domain::Contract>> getContractById(const std::string& id) const override;
    void removeContract(const std::string& id) override;
    void updateContract(const std::shared_ptr<core::domain::Contract>& contract) override;

    void upsertContract(const std::shared_ptr<core::domain::Contract>& contract) override;
    void clearContracts() override;

    std::vector<std::shared_ptr<core::domain::Contract>> getContractsForActor(const std::string& actorId) const override;
    std::vector<std::shared_ptr<core::domain::Contract>> getContractsForProperty(const std::string& propertyId) const override;
    std::vector<std::string> getActorIdsForContract(const std::string& contractId) const override;
    std::vector<std::string> getPropertyIdsForContract(const std::string& contractId) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
