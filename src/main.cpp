#include <iostream>
#include <windows.h>
#include <poppler-document.h>
#include "controllers/PropertyController.h"
#include "controllers/ExpenseController.h"
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

    // Test Poppler
    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file("test.pdf"));
    if (!doc) {
        std::cerr << "Poppler is linked correctly, but the PDF file could not be opened." << std::endl;
    }
    else {
        std::cout << "Poppler is linked correctly, and the PDF file was opened successfully." << std::endl;
    }

    return 0;
}

