#pragma once
#include "pch.h"
#include "models/PdfExtractedData.h"
#include "models/Transaction.h"

class PdfImportController {
public:
    // Extrahiert Daten aus einer PDF-Datei und gibt ein PdfExtractedData-Objekt zurück
    std::shared_ptr<PdfExtractedData> extractData(const std::string& filePath);
};
