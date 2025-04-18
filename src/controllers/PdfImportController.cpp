#include "controllers/PdfImportController.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <stdexcept>
#include <iostream>

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    // Öffnet die PDF-Datei mit Poppler
    auto document = poppler::document::load_from_file(filePath);
    if (!document) {
        throw std::runtime_error("Failed to open PDF file: " + filePath);
    }

    // Erstellt ein PdfExtractedData-Objekt für die extrahierten Daten
    auto data = std::make_shared<PdfExtractedData>(filePath);

    // Iteriert über alle Seiten der PDF-Datei
    for (int i = 0; i < document->pages(); ++i) {
        auto page = document->create_page(i);
        if (page) {
            // Extrahiert den Text der Seite
            auto text = page->text();

            // Beispiel: Fügen Sie den gesamten Text als einen Block hinzu
            // (Positionen können später erweitert werden)
            data->addTextBlock(text.to_latin1(), 0, 0, 0, 0);

            // Debug-Ausgabe
            std::cout << "Extracted text from page " << i + 1 << ": " << text.to_latin1() << std::endl;
        }
    }

    return data;
}