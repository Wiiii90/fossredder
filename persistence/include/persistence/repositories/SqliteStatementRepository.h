#pragma once

#include "core/repositories/IStatementRepository.h"
#include <memory>
#include <string>

class SqliteStatementRepository : public IStatementRepository {
public:
    explicit SqliteStatementRepository(const std::string& dbPath);
    ~SqliteStatementRepository() override;

    void addStatement(const std::shared_ptr<Statement>& statement) override;
    std::vector<std::shared_ptr<Statement>> getStatements() const override;
    std::optional<std::shared_ptr<Statement>> getStatementById(const std::string& id) const override;
    void removeStatement(const std::string& id) override;
    void updateStatement(const std::shared_ptr<Statement>& statement) override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
