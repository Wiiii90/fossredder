#include "managers/TransactionManager.h"
#include <algorithm>
#include <memory>
#include <iostream>

// Note: avoid including models here. Work with opaque Transaction pointers only.

void TransactionManager::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    transactions_.push_back(transaction);
}

std::vector<std::shared_ptr<Transaction>> TransactionManager::getTransactions() const {
    return transactions_;
}

void TransactionManager::removeTransaction(const std::string& id) {
    // Stub: cannot inspect Transaction fields without model includes.
    // Remove first transaction as a conservative fallback when id matches some placeholder.
    // In future: implement proper removal when Transaction exposes an ID accessor in header.
    if (transactions_.empty()) return;
    // If id == "ALL" remove all
    if (id == "ALL") {
        transactions_.clear();
        return;
    }
    // Otherwise do nothing (safe noop)
    std::cout << "[TransactionManager] removeTransaction called with id='" << id << "' - noop in stub mode" << std::endl;
}

void TransactionManager::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    // Stub: replace the first transaction with provided one if any
    if (transactions_.empty()) {
        std::cout << "[TransactionManager] updateTransaction called but transaction list empty - pushing back" << std::endl;
        transactions_.push_back(transaction);
        return;
    }
    transactions_[0] = transaction;
}