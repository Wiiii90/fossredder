#pragma once
#include <string>

namespace llama_prompts {
    // Prompt für die OCR-Bereinigung
    inline const std::string cleaningPromptSystem = "Du bist ein Textbereinigungs-Tool für OCR-Texte.";
    inline const std::string cleaningPromptUser =
        "Entferne überflüssige oder falsche Leerzeichen und fasse auseinandergerissene Wörter korrekt zusammen. "
        "Ersetze Buchstaben 'O' durch Ziffern '0' in Nummern und Codes, falls eindeutig. "
        "Gib ausschließlich den bereinigten Text aus.\nText: ";

    // Prompt für die Feldextraktion
    inline const std::string extractionPromptSystem = "Du bist ein Extraktions-Tool für Banktransaktionsdetails.";
    inline const std::string extractionPromptUserHeader =
        "Extrahiere aus dem folgenden Banktransaktionsdetails alle relevanten Felder zu einer Banktransaktion. "
        "Erkenne Felder anhand typischer Begriffe, Muster und Positionen, auch wenn sie nicht explizit als 'Feldname:' markiert sind. "
        "Antworte ausschließlich mit einer Liste von Tupeln im Format (Feldname, Wert) entsprechend der Struktur folgender Beispiele:\n\n";
}
