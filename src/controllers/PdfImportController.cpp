#include "pch.h"
#include "models/PdfExtractedData.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Page.h"
#include "models/Transaction.h"
#include "models/BookingGroup.h"
#include "views/ConsoleView.h"
#include "controllers/PdfImportController.h"
#include "poppler/IPdfRenderer.h"
#include "ocr/IOcrEngine.h"

PdfImportController::PdfImportController(std::shared_ptr<IOcrEngine> ocrEngine, std::shared_ptr<IPdfRenderer> pdfRenderer)
    : m_ocrEngine(std::move(ocrEngine)), m_pdfRenderer(std::move(pdfRenderer)) {}

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

    for (size_t i = 0; i < imageFiles.size(); ++i) {
        try {
            std::string xmlContent = m_ocrEngine->recognizeAltoXml(imageFiles[i], tessdataPath);
            auto page = std::make_shared<Page>(xmlContent, static_cast<int>(i + 1));
            std::vector<std::string> headerKeywords = {
                "Angaben zu den Umsätzen", "Valuta", "zu Ihren Lasten", "zu Ihren Gunsten"
            };
            
            // Extract headers
            std::vector<Header> headers = page->extractHeaders(headerKeywords);
            
            // Debug: Count blocks before cropping
            std::cout << "[DEBUG] Before cropping: " << page->getBlocks().size() << " blocks on page " << page->getIndex() << std::endl;
            
            // Calculate min header Y position (top of all headers)
            int minHeaderY = std::numeric_limits<int>::max();
            for (const auto& h : headers) {
                minHeaderY = std::min(minHeaderY, h.getY1());
            }
            
            // Calculate max header Y2 position (bottom of all headers)
            int maxHeaderY2 = 0;
            for (const auto& h : headers) {
                // Assuming header height of about 20px
                int y2 = h.getY1() + 20;
                maxHeaderY2 = std::max(maxHeaderY2, y2);
            }
            
            // Crop blocks at header bottom line (single step)
            if (maxHeaderY2 > 0) {
                page->crop(Page::CropDirection::TOP, maxHeaderY2);
            }
            
            // Debug: Count blocks after cropping
            std::cout << "[DEBUG] After cropping: " << page->getBlocks().size() << " blocks on page " << page->getIndex() << std::endl;
            
            // Crop from the bottom based on footer keywords
            std::vector<std::string> footerKeywords = {
                "Folgeseite", "Neuer Kontostand", "Guthaben sind als Einlagen"
            };

            int minFooterY = std::numeric_limits<int>::max();

            // Search in all blocks for footer keywords
            for (const auto& block : page->getBlocks()) {
                std::string text = block->getFormattedText();
                for (const auto& keyword : footerKeywords) {
                    if (text.find(keyword) != std::string::npos) {
                        // Found a footer keyword, use this block's Y position
                        minFooterY = std::min(minFooterY, block->getY1());
                        std::cout << "[DEBUG] Found footer keyword: '" << keyword 
                                  << "' at Y=" << block->getY1() << std::endl;
                        break;
                    }
                }
            }

            // If a footer keyword was found, crop everything below it
            if (minFooterY < std::numeric_limits<int>::max()) {
                page->crop(Page::CropDirection::BOTTOM, minFooterY);
                std::cout << "[DEBUG] Cropped bottom at Y=" << minFooterY << std::endl;
            }
            
            // Now assign the cropped blocks to headers
            std::vector<std::shared_ptr<Block>> pageBlocks = page->getBlocks();
            Header::assignBlocks(headers, pageBlocks);

            // Debug: Count assigned blocks
            int totalAssignedBlocks = 0;
            for (const auto& h : headers) {
                totalAssignedBlocks += h.getBlocks().size();
                std::cout << "[DEBUG] Header '" << h.getName() << "' has " << h.getBlocks().size() << " blocks" << std::endl;
            }
            std::cout << "[DEBUG] Total assigned blocks: " << totalAssignedBlocks << std::endl;
            
            // Sort blocks within each header
            for (auto& h : headers) {
                h.sortBlocks();
            }

            /*std::vector<Transaction> pageTransactions = BookingGroup::extractTransactions(headers);
            allTransactions.insert(allTransactions.end(), pageTransactions.begin(), pageTransactions.end());*/

            allPages.push_back(page);
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }

    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allPages, allTransactions);
    return pdfData;
}