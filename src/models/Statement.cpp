#include "models/Statement.h"
#include "models/Transaction.h"

Statement::Statement(const std::string& period, std::shared_ptr<Property> property)
	: period(period), property(std::move(property)) {
}

const std::string& Statement::getPeriod() const {
	return period;
}

std::shared_ptr<Property> Statement::getProperty() const {
	return property;
}

const std::vector<std::shared_ptr<Transaction>>& Statement::getTransactions() const {
	return transactions;
}

void Statement::addTransaction(const std::shared_ptr<Transaction>& transaction) {
	transactions.push_back(transaction);
}

double Statement::getTotal() const {
	double total = 0.0;
	for (const auto& t : transactions) {
		if (t) total += t->getAmount();
	}
	return total;
}