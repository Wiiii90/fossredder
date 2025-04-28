#pragma once
#include "pch.h"
#include "models/PdfExtractedData.h"

namespace fs = std::filesystem;

class PdfImportController {
public:
    PdfImportController(); // Constructor
    std::shared_ptr<PdfExtractedData> extractData(const std::string& filePath);
};