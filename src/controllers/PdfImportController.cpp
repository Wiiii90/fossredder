#include "pch.h"
#include "models/PdfExtractedData.h"
#include "models/Header.h"
#include "models/Page.h"
#include "models/Transaction.h"
#include "views/ConsoleView.h"
#include "controllers/PdfImportController.h"
#include "poppler/IPdfRenderer.h"
#include "ocr/IOcrEngine.h"

PdfImportController::PdfImportController(std::shared_ptr<IOcrEngine> ocrEngine, std::shared_ptr<IPdfRenderer> pdfRenderer)
    : m_ocrEngine(std::move(ocrEngine)), m_pdfRenderer(std::move(pdfRenderer)) {}

namespace {
    
    bool fileExists(const std::string& filePath) {
        return std::filesystem::exists(filePath);
    }

    std::string convertPdfToImages(const std::string& filePath, const std::string& outputPrefix) {
        std::string command = "pdftoppm -png -r 256 \"" + filePath + "\" " + outputPrefix;
        if (std::system(command.c_str()) != 0) {
            throw std::runtime_error("Failed to execute pdftoppm. Ensure it is installed and in your PATH.");
        }
        return outputPrefix;
    }

    std::vector<std::string> getGeneratedImageFiles(const std::string& outputPrefix) {
        std::vector<std::string> imageFiles;
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            std::string fileName = entry.path().filename().string();
            if (fileName.find(outputPrefix) == 0 && fileName.find(".png") != std::string::npos) {
                imageFiles.push_back(fileName);
            }
        }
        return imageFiles;
    }
}

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    ConsoleView consoleView;
    if (!std::filesystem::exists(filePath)) {
        consoleView.displayError("PDF file does not exist: " + filePath);
        throw std::runtime_error("PDF file does not exist: " + filePath);
    }
    const std::string tessdataPath = "C:/coding/fossredder/res/tessdata/";
    const std::string outputPrefix = "page";

    std::vector<std::string> imageFiles = m_pdfRenderer->renderToImages(filePath, outputPrefix);

    std::vector<std::shared_ptr<Page>> allPages;
    std::vector<Transaction> allTransactions;

    for (const auto& imageFile : imageFiles) {
        try {
            std::string xmlContent = m_ocrEngine->recognizeAltoXml(imageFile, tessdataPath);
            auto page = std::make_shared<Page>(xmlContent);
            std::vector<std::string> headerKeywords = {
                "Angaben zu den Umsätzen", "Valuta", "zu Ihren Lasten", "zu Ihren Gunsten"
            };
            std::vector<Header> headers = page->extractHeaders(headerKeywords);

            std::vector<std::shared_ptr<Block>> pageBlocks = page->getBlocks();
            Header::assignBlocks(headers, pageBlocks, page->getWidth());

            for (auto& h : headers) {
                h.sortBlocks();
            }

            std::vector<Transaction> pageTransactions = Header::extractTransactions(headers);
            allTransactions.insert(allTransactions.end(), pageTransactions.begin(), pageTransactions.end());

            allPages.push_back(page);
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }

    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allPages, allTransactions);
    return pdfData;
}