#include "managers/StatementManager.h"
#include "models/Statement.h"
#include "models/Transaction.h"
#include <algorithm>
#include <memory>

void StatementManager::addStatement(const std::shared_ptr<Statement>& statement) {
    statements_.push_back(statement);
}

std::vector<std::shared_ptr<Statement>> StatementManager::getStatements() const {
    return statements_;
}

void StatementManager::removeStatement(const std::string& period) {
    statements_.erase(
        std::remove_if(statements_.begin(), statements_.end(),
            [&](const std::shared_ptr<Statement>& s) { return s && s->getPeriod() == period; }),
        statements_.end());
}

void StatementManager::updateStatement(const std::shared_ptr<Statement>& statement) {
    for (auto& s : statements_) {
        if (s && s->getPeriod() == statement->getPeriod()) {
            s = statement;
            break;
        }
    }
}