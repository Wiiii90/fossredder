#pragma once
#include <string>
#include <vector>

// Struktur für einen Textblock mit Position
struct TextBlock {
    std::string text;
    double x, y, width, height; // Position und Größe des Textblocks

    TextBlock(const std::string& text, double x, double y, double width, double height)
        : text(text), x(x), y(y), width(width), height(height) {}
};

// Klasse für die extrahierten PDF-Daten
class PdfExtractedData {
public:
    std::vector<TextBlock> textBlocks; // Liste der Textblöcke
    std::string sourceFile;            // Name der PDF-Datei

    PdfExtractedData(const std::string& file) : sourceFile(file) {}

    // Methode zum Hinzufügen eines Textblocks
    void addTextBlock(const std::string& text, double x, double y, double width, double height) {
        textBlocks.emplace_back(text, x, y, width, height);
    }
};
