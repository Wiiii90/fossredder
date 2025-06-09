#pragma once
#include <memory>
#include <vector>
#include <string>

class Statement;

class IStatementManager {
public:
    virtual ~IStatementManager() = default;
    virtual void addStatement(const std::shared_ptr<Statement>& statement) = 0;
    virtual std::vector<std::shared_ptr<Statement>> getStatements() const = 0;
    virtual void removeStatement(const std::string& id) = 0;
    virtual void updateStatement(const std::shared_ptr<Statement>& statement) = 0;
};