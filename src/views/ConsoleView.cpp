#include "pch.h"
#include "views/ConsoleView.h"
#include "models/Transaction.h"
#include "models/PdfExtractedData.h"

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    std::cout << "[DEBUG] ConsoleView::displayPdfData called." << std::endl;
    std::cout << "Displaying PDF Data (seitenweise):" << std::endl;
    int pageNum = 1;
    for (const auto& page : data->getPages()) {
        std::cout << "=============================" << std::endl;
        std::cout << "Seite " << pageNum++ << std::endl;
        int blockNum = 1;
        for (const auto& block : page->getBlocks()) {
            std::cout << "-----------------------------\n";
            std::cout << "Block " << blockNum++ << ": ";
            std::cout << "Pos (" << block->getX1() << "," << block->getY1() << "), "
                      << "Größe " << block->getWidth() << "x" << block->getHeight() << "\n";
            std::cout << "Text:\n" << block->getFormattedText() << "\n";
            std::cout << "-----------------------------\n";
        }
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