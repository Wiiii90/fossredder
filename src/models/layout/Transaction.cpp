#include "pch.h"
#include "models/layout/Transaction.h"
#include <sstream>

Transaction::Transaction(std::string bookingDate,
	std::string valueDate,
	double amount,
	std::string actor,
	std::map<std::string, std::string> metadata)
	: bookingDate(std::move(bookingDate)), valueDate(std::move(valueDate)), amount(amount), actor(std::move(actor)), metadata(std::move(metadata)) {
}

bool Transaction::isDebit() const noexcept { return amount < 0.0; }
bool Transaction::isCredit() const noexcept { return amount > 0.0; }

std::string Transaction::toString() const {
	std::ostringstream oss;
	oss << "bookingDate=" << bookingDate << ", valueDate=" << valueDate << ", amount=" << amount << ", actor=" << actor;
	return oss.str();
}
