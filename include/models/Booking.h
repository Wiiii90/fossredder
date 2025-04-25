#pragma once
#include "pch.h"
#include "Transaction.h"

class Booking {
public:
    std::string bookingDate; // Buchungsdatum
    std::vector<Transaction> transactions; // Liste der zugehörigen Transaktionen

    Booking(const std::string& bookingDate) : bookingDate(bookingDate) {}

    // Methode zum Hinzufügen einer Transaktion
    void addTransaction(const Transaction& transaction) {
        transactions.push_back(transaction);
    }
};