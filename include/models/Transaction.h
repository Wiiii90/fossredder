#pragma once
#include <string>
#include "models/BookingGroup.h" 

class BookingGroup;

class Transaction {
public:
    Transaction(const BookingGroup* bookingGroup);

    std::string bookingDate;
    std::string valutaDate;
    std::string details;
    double amount;
};