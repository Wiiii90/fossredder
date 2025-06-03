#include "pch.h"
#include "views/ConsoleView.h"
#include "models/Transaction.h"
#include "models/PdfExtractedData.h"
#include "models/Page.h"
#include "models/BookingGroup.h"
#include <iomanip>
#include <sstream>

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    if (!data) {
        std::cout << "[Fehler] Keine Daten zum Anzeigen.\n";
        return;
    }

    // Übersicht zu den Seiten
    const auto& pages = data->getPages();
    std::cout << "\n====================[ PDF SEITEN ]====================\n";
    std::cout << "Anzahl Seiten: " << pages.size() << "\n";
    for (size_t i = 0; i < pages.size(); ++i) {
        const auto& page = pages[i];
        std::cout << "  Seite #" << (i + 1)
            << " | Größe: " << page->getWidth() << "x" << page->getHeight()
            << " | Blöcke: " << page->getBlocks().size()
            << " | Header: " << page->getHeaders().size() << "\n";
    }
    std::cout << "======================================================\n";

    // Übersicht zu den Buchungsgruppen und Transaktionen
    const auto& bookingGroups = data->getBookingGroups();
    std::cout << "\n====================[ BUCHUNGSGRUPPEN & TRANSAKTIONEN ]====================\n";
    std::cout << "Anzahl Buchungsgruppen: " << bookingGroups.size() << "\n";
    int groupIdx = 1;
    for (const auto& group : bookingGroups) {
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  Buchungsgruppe #" << groupIdx++ << "\n";
        std::cout << "╠════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  Buchungsdatum      : " << group.getBookingDate() << "\n";
        std::cout << "║  Details-Blocks     : " << group.getDetailsBlocks().size() << "\n";
        std::cout << "║  Valuta-Blocks      : " << group.getValutaBlocks().size() << "\n";
        std::cout << "║  Debit-Blocks       : " << group.getDebitBlocks().size() << "\n";
        std::cout << "║  Credit-Blocks      : " << group.getCreditBlocks().size() << "\n";
        std::cout << "║  Transaktionen      : " << group.getTransactions().size() << "\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════════╝\n";

        int txIdx = 1;
        for (const auto& t : group.getTransactions()) {
            std::cout << "  ┌────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "  │  Transaktion #" << txIdx++ << "\n";
            std::cout << "  ├────────────────────────────────────────────────────────────────────┤\n";
            std::cout << "  │  Buchungsdatum : " << t.getBookingDate() << "\n";
            std::cout << "  │  Valuta        : " << t.valutaDate << "\n";

            // Betrag ggf. Zeilenumbrüche entfernen
            std::string amountText = t.amountText;
            amountText.erase(std::remove(amountText.begin(), amountText.end(), '\n'), amountText.end());
            std::cout << "  │  Betrag        : " << std::fixed << std::setprecision(2) << t.amount << " (" << amountText << ")\n";

            // Details
            std::cout << "  │  Details       :";
            std::istringstream detailsStream(t.details);
            std::string line;
            bool hasDetails = false;
            while (std::getline(detailsStream, line)) {
                if (!line.empty()) {
                    if (!hasDetails) std::cout << "\n";
                    std::cout << "  │    " << line << "\n";
                    hasDetails = true;
                }
            }
            if (!hasDetails) {
                std::cout << " [keine Details]\n";
            }
            std::cout << "  └────────────────────────────────────────────────────────────────────┘\n";
        }
        if (group.getTransactions().empty()) {
            std::cout << "  [Keine Transaktionen in dieser Buchungsgruppe]\n";
        }
        std::cout << "\n";
    }
    std::cout << "============================================================================\n";
}

void ConsoleView::displayError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}