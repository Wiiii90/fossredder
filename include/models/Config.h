#pragma once
#include <string>
#include <optional>

// Beispielhafte Felder für die Konfiguration
struct Config {
    std::string ocrLanguage = "deu";
    std::string bankTemplate = "default";
    int ocrDpi = 300;
    // Weitere Felder nach Bedarf
};