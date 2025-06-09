#pragma once
#include "managers/IStatementManager.h"

class StatementManager : public IStatementManager {
public:
    void addStatement(const std::shared_ptr<Statement>& statement) override;
    std::vector<std::shared_ptr<Statement>> getStatements() const override;
    void removeStatement(const std::string& id) override;
    void updateStatement(const std::shared_ptr<Statement>& statement) override;
private:
    std::vector<std::shared_ptr<Statement>> statements_;
};