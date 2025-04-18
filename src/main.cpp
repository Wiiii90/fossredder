#include <iostream>
#include <windows.h>
#include <poppler-document.h>
#include "controllers/PropertyController.h"
#include "controllers/ExpenseController.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    PropertyController propertyController;
    ExpenseController expenseController;

    // Beispiel: Immobilien hinzufügen
    auto property1 = std::make_shared<Property>("Immobilie 1", "Musterstraße 1", "Bürogebäude");
    auto property2 = std::make_shared<Property>("Immobilie 2", "Beispielweg 2", "Lagerhalle");
    propertyController.addProperty(property1->name, property1->address, property1->description);
    propertyController.addProperty(property2->name, property2->address, property2->description);

    // Beispiel: Kategorien erstellen
    auto category1 = std::make_shared<Category>("Reparatur", true);
    auto category2 = std::make_shared<Category>("Versicherung", false);

    // Beispiel: Ausgaben hinzufügen
    expenseController.addExpense(500.0, "2025-04-18", category1, property1, true);
    expenseController.addExpense(300.0, "2025-04-18", category2, property2, false);

    // Immobilien anzeigen
    ConsoleView::displayProperties(propertyController.getProperties());

    // Ausgaben anzeigen
    ConsoleView::displayExpenses(expenseController.getExpenses());

    // PDF-Verarbeitung
    PdfImportController pdfController;
    try {
        auto pdfData = pdfController.extractData("C:\\coding\\fossredder\\test.pdf"); // Beispiel-PDF-Datei
        ConsoleView::displayPdfData(pdfData); // Extrahierte Daten anzeigen
    }
    catch (const std::exception& e) {
        std::cerr << "Error during PDF processing: " << e.what() << std::endl;
    }

    return 0;
}
