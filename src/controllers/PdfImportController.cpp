#define NOMINMAX
#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"
#include "models/PdfExtractedData.h"
#include "models/Header.h"
#include "models/Page.h"
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <limits>
#include <cmath>
#include <sstream>
#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <cctype>
#include "models/Transaction.h"

PdfImportController::PdfImportController(std::shared_ptr<IOcrEngine> ocrEngine)
    : m_ocrEngine(std::move(ocrEngine)) {}

namespace {
    
    bool fileExists(const std::string& filePath) {
        return std::filesystem::exists(filePath);
    }

    std::string convertPdfToImages(const std::string& filePath, const std::string& outputPrefix) {
        std::string command = "pdftoppm -png -r 600 \"" + filePath + "\" " + outputPrefix;
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

} // namespace

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    ConsoleView consoleView;
    if (!fileExists(filePath)) {
        consoleView.displayError("PDF file does not exist: " + filePath);
        throw std::runtime_error("PDF file does not exist: " + filePath);
    }
    const std::string tessdataPath = "C:/coding/fossredder/res/tessdata/";
    const std::string outputPrefix = "page";
    convertPdfToImages(filePath, outputPrefix);

    std::vector<std::shared_ptr<Page>> allPages;
    std::vector<Header> allHeaders;

    auto imageFiles = getGeneratedImageFiles(outputPrefix);
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

            allHeaders.insert(allHeaders.end(), headers.begin(), headers.end());
            allPages.push_back(page);
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }

    std::vector<Transaction> transactions = Header::extractTransactions(allHeaders);

    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allPages, transactions);
    return pdfData;
}