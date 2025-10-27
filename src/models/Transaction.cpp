#include "pch.h"
#include "models/Transaction.h"
#include "models/BookingGroup.h"

Transaction::Transaction() : amount(0.0) {}

Transaction::Transaction(const std::string& bookingDate,
	const std::string& valutaDate,
	const std::string& details,
	const std::string& amountText)
	: bookingDate(bookingDate),
	valutaDate(valutaDate),
	details(details),
	amountText(amountText),
	amount(0.0)
{
	// Parse numeric amount from the textual representation immediately
	amount = parseAmount(this->amountText);
}

Transaction::Transaction(const Transaction& other)
	: bookingDate(other.bookingDate),
	valutaDate(other.valutaDate),
	details(other.details),
	amountText(other.amountText),
	amount(other.amount)
{
}

double Transaction::parseAmount(const std::string& text) const
{
	std::string s;
	for (char c : text) {
		if (!std::isspace(static_cast<unsigned char>(c)))
			s += c;
	}
	bool negative = false;
	if (!s.empty() && (s.back() == '-' || s.front() == '-')) {
		negative = true;
		if (s.back() == '-') s.pop_back();
		if (!s.empty() && s.front() == '-') s.erase(0, 1);
	}
	size_t lastComma = s.rfind(',');
	size_t lastDot = s.rfind('.');
	if (lastComma != std::string::npos && (lastDot == std::string::npos || lastComma > lastDot)) {
		s.erase(std::remove(s.begin(), s.end(), '.'), s.end());
		std::replace(s.begin(), s.end(), ',', '.');
	}
	else if (lastDot != std::string::npos && (lastComma == std::string::npos || lastDot > lastComma)) {
		s.erase(std::remove(s.begin(), s.end(), ','), s.end());
	}
	double value = 0.0;
	try {
		value = std::stod(s);
	}
	catch (...) {
		value = 0.0;
	}
	if (negative) value = -value;
	return value;
}

double Transaction::getAmount() const {
	return amount;
}

std::string Transaction::getBookingDate() const {
	return bookingDate;
}