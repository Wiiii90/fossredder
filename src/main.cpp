#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    PdfImportController pdfController;
    ConsoleView consoleView;

    try {
        auto extractedData = pdfController.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");
        consoleView.displayTransactions(extractedData->getTransactions());
    }
    catch (const std::exception& e) {
        consoleView.displayError("Error during PDF processing: " + std::string(e.what()));
    }

    return 0;
}