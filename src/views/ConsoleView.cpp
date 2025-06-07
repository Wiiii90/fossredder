#include "pch.h"
#include "views/ConsoleView.h"
#include "models/Transaction.h"
#include "models/PdfExtractedData.h"
#include "models/Page.h"
#include "models/BookingGroup.h"

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    if (!data) {
        std::cout << "[Fehler] Keine Daten zum Anzeigen.\n";
        return;
    }

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

            std::istringstream valutaStream(t.valutaDate);
            std::string valutaLine;
            bool firstValuta = true;
            while (std::getline(valutaStream, valutaLine)) {
                if (firstValuta) {
                    std::cout << "  │  Valuta        : " << valutaLine << "\n";
                    firstValuta = false;
                }
                else if (!valutaLine.empty()) {
                    std::cout << "  │                 " << valutaLine << "\n";
                }
            }
            std::cout << "  │\n";

            std::string amountText = t.amountText;
            amountText.erase(std::remove(amountText.begin(), amountText.end(), '\n'), amountText.end());
            std::cout << "  │  Betrag        : " << std::fixed << std::setprecision(2) << t.amount << " ( " << amountText << ")\n";

            std::cout << "  │  Details       :";
            std::istringstream detailsStream(t.details);
            std::string line;
            std::vector<std::string> compactLines;
            while (std::getline(detailsStream, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                if (!line.empty()) {
                    while (line.find("  ") != std::string::npos)
                        line.replace(line.find("  "), 2, " ");
                    compactLines.push_back(line);
                }
            }
            if (compactLines.empty()) {
                std::cout << " [keine Details]\n";
            }
            else {
                std::cout << "\n";
                std::string buffer;
                for (size_t i = 0; i < compactLines.size(); ++i) {
                    const std::string& l = compactLines[i];
                    if (l.size() <= 3 && i + 1 < compactLines.size()) {
                        buffer += l + " ";
                        continue;
                    }
                    if (!buffer.empty()) {
                        buffer += l;
                        std::cout << "  │    " << buffer << "\n";
                        buffer.clear();
                    }
                    else if (l.size() <= 10 && i + 1 < compactLines.size() && compactLines[i + 1].size() <= 10) {
                        buffer = l + " ";
                        continue;
                    }
                    else {
                        std::cout << "  │    " << l << "\n";
                    }
                }
                if (!buffer.empty()) {
                    std::cout << "  │    " << buffer << "\n";
                }
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