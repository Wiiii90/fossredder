#pragma once
#include <string>
#include <memory>
#include "models/PdfExtractedData.h"

class PdfImportController {
public:
    // Extrahiert Daten aus einer PDF-Datei und gibt ein PdfExtractedData-Objekt zurück
    std::shared_ptr<PdfExtractedData> extractData(const std::string& filePath);
};
