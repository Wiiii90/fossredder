#pragma once
#include "pch.h"
#include "Booking.h"

// Klasse für die extrahierten PDF-Daten
class PdfExtractedData {
public:
    std::string sourceFile;            // Name der PDF-Datei
    std::vector<std::shared_ptr<Booking>> bookings; // Liste der Buchungen

    PdfExtractedData(const std::string& file) : sourceFile(file) {}

    // Methode zum Hinzufügen einer Buchung
    void addBooking(const std::shared_ptr<Booking>& booking) {
        bookings.push_back(booking);
    }
};