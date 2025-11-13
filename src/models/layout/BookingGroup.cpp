#include "pch.h"
#include "models/layout/BookingGroup.h"

BookingGroup::BookingGroup(std::vector<Transaction> txs) : transactions(std::move(txs)) {
    if (transactions.empty()) throw std::invalid_argument("BookingGroup requires at least one Transaction");
}

void BookingGroup::addTransaction(Transaction t) {
    transactions.push_back(std::move(t));
}

bool BookingGroup::removeTransactionAt(size_t idx) {
    if (idx >= transactions.size()) return false;
    if (transactions.size() <= 1) return false;
    transactions.erase(transactions.begin() + idx);
    return true;
}

double BookingGroup::totalAmount() const noexcept {
    double sum = 0.0;
    for (const auto &t : transactions) sum += t.amount;
    return sum;
}

std::optional<size_t> BookingGroup::findIndexById(const std::string& id) const noexcept {
    if (id.empty()) return std::nullopt;
    for (size_t i = 0; i < transactions.size(); ++i) {
        auto it = transactions[i].metadata.find("id");
        if (it != transactions[i].metadata.end() && it->second == id) return i;
    }
    return std::nullopt;
}

