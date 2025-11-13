#include "pch.h"
#include "models/layout/Statement.h"

Statement::Statement(std::vector<BookingGroup> groups) : bookingGroups(std::move(groups)) {}

void Statement::addBookingGroup(BookingGroup g) {
	bookingGroups.push_back(std::move(g));
}

bool Statement::removeBookingGroupAt(size_t idx) {
	if (idx >= bookingGroups.size()) return false;
	bookingGroups.erase(bookingGroups.begin() + idx);
	return true;
}

double Statement::totalAmount() const noexcept {
	double s = 0.0;
	for (const auto& g : bookingGroups) s += g.totalAmount();
	return s;
}

std::vector<Transaction> Statement::flattenTransactions() const {
	std::vector<Transaction> out;
	for (const auto& g : bookingGroups) {
		const auto& t = g.getTransactions();
		out.insert(out.end(), t.begin(), t.end());
	}
	return out;
}

std::optional<size_t> Statement::findGroupByTitle(const std::string& title) const noexcept {
	if (title.empty()) return std::nullopt;
	for (size_t i = 0; i < bookingGroups.size(); ++i) {
		if (bookingGroups[i].title == title) return i;
	}
	return std::nullopt;
}
