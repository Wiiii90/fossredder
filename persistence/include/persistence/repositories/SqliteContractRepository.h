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

    void addContract(const std::shared_ptr<Contract>& contract) override;
    std::vector<std::shared_ptr<Contract>> getContracts() const override;
    std::optional<std::shared_ptr<Contract>> getContractById(const std::string& id) const override;
    void removeContract(const std::string& id) override;
    void updateContract(const std::shared_ptr<Contract>& contract) override;

    void upsertContract(const std::shared_ptr<Contract>& contract) override;
    void clearContracts() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
