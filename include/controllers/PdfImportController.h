#pragma once
#include <memory>
#include <string>

class IOcrEngine;
class IPdfRenderer;
class PdfExtractedData;

class PdfImportController {
public:
    PdfImportController(std::shared_ptr<IOcrEngine> ocrEngine, std::shared_ptr<IPdfRenderer> pdfRenderer);
    std::shared_ptr<PdfExtractedData> extractData(const std::string& filePath);

private:
    std::shared_ptr<IOcrEngine> m_ocrEngine;
    std::shared_ptr<IPdfRenderer> m_pdfRenderer;
};