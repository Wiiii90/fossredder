#include "pch.h"
#include "views/ConsoleView.h"
#include "models/Transaction.h"
#include "models/PdfExtractedData.h"

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    std::cout << "[DEBUG] ConsoleView::displayPdfData called." << std::endl;
    std::cout << "Displaying PDF Data:" << std::endl;
    for (const auto& element : data->getElements()) {
        std::cout << "-----------------------------\n";
        std::cout << "Element:\n";
        std::cout << "Position: (" << element->getX1() << ", " << element->getY1() << ")\n";
        std::cout << "Size: " << element->getWidth() << "x" << element->getHeight() << "\n";
        std::cout << "Text:\n" << element->getFormattedText() << "\n";
        std::cout << "-----------------------------\n";
    }
}

void ConsoleView::displayTransactions(const std::vector<Transaction>& transactions) {
    std::cout << "=== Extracted Transactions ===" << std::endl;
    std::cout << "Idx | BookingDate | ValutaDate | Actor | Description | Amount | Type" << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;
    int idx = 1;
    for (const auto& t : transactions) {
        std::cout << idx++ << " | "
            << t.bookingDate << " | "
            << t.valutaDate << " | "
            << t.actor << " | "
            << t.description << " | "
            << t.amount << " | "
            << (t.isDebit ? "Debit" : "Credit") << std::endl;
    }
    std::cout << "---------------------------------------------------------------------" << std::endl;
}

void ConsoleView::displayError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}