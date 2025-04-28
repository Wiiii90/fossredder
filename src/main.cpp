#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // PDF Processing
    PdfImportController pdfController;
    ConsoleView consoleView;

    try {
        // Extract data from the PDF
        auto extractedData = pdfController.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");

        // Display the extracted blocks
        consoleView.displayPdfData(extractedData);
    }
    catch (const std::exception& e) {
        consoleView.displayError("Error during PDF processing: " + std::string(e.what()));
    }

    return 0;
}