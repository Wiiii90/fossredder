#include "core/pch.h"
#include "core/models/Statement.h"

Statement::Statement(std::vector<Transaction> transactions) : transactions(std::move(transactions)) {}

void Statement::addTransaction(const Transaction& t) {
    transactions.push_back(t);
}

bool Statement::removeTransaction(const Transaction& t) {
    auto it = std::find(transactions.begin(), transactions.end(), t);
    if (it == transactions.end()) return false;
    transactions.erase(it);
    return true;
}
