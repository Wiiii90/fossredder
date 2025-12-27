#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

class Statement;

class IStatementRepository {
public:
    virtual ~IStatementRepository() = default;

    virtual void addStatement(const std::shared_ptr<Statement>& statement) = 0;
    virtual std::vector<std::shared_ptr<Statement>> getStatements() const = 0;
    virtual std::optional<std::shared_ptr<Statement>> getStatementById(const std::string& id) const = 0;
    virtual void removeStatement(const std::string& id) = 0;
    virtual void updateStatement(const std::shared_ptr<Statement>& statement) = 0;

    virtual void upsertStatement(const std::shared_ptr<Statement>& statement) = 0;
    virtual void clearStatements() = 0;
};
