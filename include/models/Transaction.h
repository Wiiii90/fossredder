#pragma once
#include "pch.h"

class Transaction {
public:
    std::string bookingDate;  // Buchungsdatum
    std::string valutaDate;   // Valuta
    std::string actor;        // Akteur (z. B. Alte Leipziger Versicherung)
    std::string description;  // Beschreibung (z. B. Mandatsref, Gl�ubiger-ID)
    double amount;            // Betrag
    bool isDebit;             // Belastung (true) oder Gutschrift (false)

    Transaction(const std::string& bookingDate, const std::string& valutaDate,
        const std::string& actor, const std::string& description,
        double amount, bool isDebit)
        : bookingDate(bookingDate), valutaDate(valutaDate), actor(actor),
        description(description), amount(amount), isDebit(isDebit) {
    }
};
