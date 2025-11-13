#include "managers/StatementManager.h"
#include <algorithm>
#include <iostream>
#include <memory>

void StatementManager::addStatement(const std::shared_ptr<Statement>& statement) {
    statements_.push_back(statement);
}

std::vector<std::shared_ptr<Statement>> StatementManager::getStatements() const {
    return statements_;
}

void StatementManager::removeStatement(const std::string& period) {
    // Stub: no access to Statement internals here. Support special command "ALL" to clear.
    if (period == "ALL") {
        statements_.clear();
        return;
    }
    std::cout << "[StatementManager] removeStatement called with period='" << period << "' - noop in stub mode" << std::endl;
}

void StatementManager::updateStatement(const std::shared_ptr<Statement>& statement) {
    // Stub: replace first statement if exists, otherwise push back
    if (statements_.empty()) { statements_.push_back(statement); return; }
    statements_[0] = statement;
}