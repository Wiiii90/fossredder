#pragma once
#include <memory>
#include <string>
#include "pch.h"
#include "models/PdfExtractedData.h"
#include "controllers/IOcrEngine.h"

namespace fs = std::filesystem;

class PdfImportController {
public:
    explicit PdfImportController(std::shared_ptr<IOcrEngine> ocrEngine);
    std::shared_ptr<PdfExtractedData> extractData(const std::string& filePath);
private:
    std::shared_ptr<IOcrEngine> m_ocrEngine;
};