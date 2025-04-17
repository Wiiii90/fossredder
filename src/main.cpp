#include <iostream>
#include "controllers/PropertyController.h"
#include "views/ConsoleView.h"



int main() {
    PropertyController propertyController;

    // Beispiel: Immobilien hinzufügen
    propertyController.addProperty("Immobilie 1", "Musterstraße 1", "Bürogebäude");
    propertyController.addProperty("Immobilie 2", "Beispielweg 2", "Lagerhalle");

    // Immobilien anzeigen
    ConsoleView::displayProperties(propertyController.getProperties());

    return 0;
}