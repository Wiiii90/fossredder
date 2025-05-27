#include "pch.h"
#include "models/Transaction.h"

Transaction::Transaction(const std::string& bookingDate,
                         const std::string& valutaDate,
                         const std::string& actor,
                         const std::string& description,
                         double amount,
                         bool isDebit)
    : bookingDate(bookingDate),
      valutaDate(valutaDate),
      actor(actor),
      description(description),
      amount(amount),
      isDebit(isDebit)
{}