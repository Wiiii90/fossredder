#include "pch.h"  
#include "models/Transaction.h"

Transaction::Transaction(const BookingGroup* bookingGroup)
    : bookingDate(bookingGroup->getBookingDate()),
    valutaDate(""),
    details(""),
    amount(0.0)
{}