#include "managers/TransactionManager.h"
#include "models/Transaction.h"
#include <algorithm>
#include <memory>

void TransactionManager::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    transactions_.push_back(transaction);
}

std::vector<std::shared_ptr<Transaction>> TransactionManager::getTransactions() const {
    return transactions_;
}

void TransactionManager::removeTransaction(const std::string& id) {
    transactions_.erase(
        std::remove_if(transactions_.begin(), transactions_.end(),
            [&](const std::shared_ptr<Transaction>& t) { return t && t->bookingDate == id; }),
        transactions_.end());
}

void TransactionManager::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    for (auto& t : transactions_) {
        if (t && t->bookingDate == transaction->bookingDate) {
            t = transaction;
            break;
        }
    }
}