#include "pch.h"
#include "models/BookingGroup.h"

BookingGroup::BookingGroup(const std::string& bookingDate, std::vector<Transaction>&& transactions)
    : bookingDate(bookingDate), transactions(std::move(transactions)) {
}

const std::string& BookingGroup::getBookingDate() const {
    return bookingDate;
}

const std::vector<Transaction>& BookingGroup::getTransactions() const {
    return transactions;
}

double BookingGroup::parseAmount(const std::string& text) {
    std::string clean = text;
    clean.erase(std::remove(clean.begin(), clean.end(), '.'), clean.end());
    std::replace(clean.begin(), clean.end(), ',', '.');
    try {
        return std::stod(clean);
    }
    catch (...) {
        return 0.0;
    }
}

std::vector<Transaction> Header::extractTransactions(const std::vector<Header>& headers) {

}