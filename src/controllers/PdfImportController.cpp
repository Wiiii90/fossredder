#define NOMINMAX
#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"
#include "models/PdfExtractedData.h"
#include "models/Header.h"
#include "tinyxml2.h"
#include <filesystem>
#include "models/Block.h"
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

PdfImportController::PdfImportController() {}

namespace {

    std::string trim(const std::string& s) {
        if (s.empty()) return s;
        size_t start = s.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\n\r\f\v");
        return s.substr(start, end - start + 1);
    }

    bool normalizedEquals(const std::string& a, const std::string& b) {
        UErrorCode error = U_ZERO_ERROR;
        const icu::Normalizer2* normalizer = icu::Normalizer2::getNFCInstance(error);
        if (U_FAILURE(error)) return false;
        icu::UnicodeString ua = icu::UnicodeString::fromUTF8(a);
        icu::UnicodeString ub = icu::UnicodeString::fromUTF8(b);
        icu::UnicodeString na, nb;
        normalizer->normalize(ua, na, error);
        normalizer->normalize(ub, nb, error);
        return na.caseCompare(nb, 0) == 0;
    }

    std::vector<Transaction> extractTransactions(const std::vector<Header>& headers) {
        const Header* valutaHeader = nullptr;
        const Header* descriptionHeader = nullptr;
        const Header* creditHeader = nullptr;
        const Header* debitHeader = nullptr;

        for (const auto& h : headers) {
            std::string n = h.getName();
            if (n == "Valuta") valutaHeader = &h;
            else if (n == "Angaben zu den Umsätzen") descriptionHeader = &h;
            else if (n == "zu Ihren Gunsten") creditHeader = &h;
            else if (n == "zu Ihren Lasten") debitHeader = &h;
        }

        if (!valutaHeader || !descriptionHeader || (!creditHeader && !debitHeader)) {
            std::cout << "[DEBUG] Required headers not found." << std::endl;
            return {};
        }

        const auto& valutaBlocks = valutaHeader->getBlocks();
        if (valutaBlocks.size() < 2) return {};

        std::vector<Transaction> transactions;
        for (size_t i = 1; i < valutaBlocks.size(); ++i) {
            int vpos = valutaBlocks[i]->getY1();
            std::string valuta = valutaBlocks[i]->getFormattedText();
            std::string description, actor, bookingDate;
            double amount = 0.0;
            bool isDebit = false;

            // Find description
            for (const auto& block : descriptionHeader->getBlocks()) {
                if (std::abs(block->getY1() - vpos) < 5) {
                    description = block->getFormattedText();
                    break;
                }
            }

            // Find amount in credit or debit column
            std::string amountStr;
            if (creditHeader) {
                for (const auto& block : creditHeader->getBlocks()) {
                    if (std::abs(block->getY1() - vpos) < 5) {
                        amountStr = block->getFormattedText();
                        isDebit = false;
                        break;
                    }
                }
            }
            if (amountStr.empty() && debitHeader) {
                for (const auto& block : debitHeader->getBlocks()) {
                    if (std::abs(block->getY1() - vpos) < 5) {
                        amountStr = block->getFormattedText();
                        isDebit = true;
                        break;
                    }
                }
            }

            // Parse amount (handle minus sign and German format)
            if (!amountStr.empty()) {
                std::string clean = amountStr;
                clean.erase(std::remove(clean.begin(), clean.end(), '.'), clean.end());
                std::replace(clean.begin(), clean.end(), ',', '.');
                try {
                    amount = std::stod(clean);
                    if (isDebit && amount > 0) amount = -amount;
                }
                catch (...) {
                    std::cout << "[DEBUG] Could not parse amount: " << amountStr << std::endl;
                }
            }

            if (i % 10 == 1) {
                std::cout << "[DEBUG] Transaction: valuta=" << valuta << ", amount=" << amount << ", description=" << description << std::endl;
            }

            transactions.emplace_back(bookingDate, valuta, actor, description, amount, isDebit);
        }
        return transactions;
    }

    // Find headers in the XML using the given keywords and store xmin/xmax using HPOS/WIDTH
    std::vector<Header> findHeaders(tinyxml2::XMLElement* printSpace, const std::vector<std::string>& headerKeywords) {
        std::vector<Header> headers;
        for (tinyxml2::XMLElement* composedBlock = printSpace->FirstChildElement("ComposedBlock");
            composedBlock != nullptr;
            composedBlock = composedBlock->NextSiblingElement("ComposedBlock")) {
            for (tinyxml2::XMLElement* textBlock = composedBlock->FirstChildElement("TextBlock");
                textBlock != nullptr;
                textBlock = textBlock->NextSiblingElement("TextBlock")) {
                for (tinyxml2::XMLElement* textLine = textBlock->FirstChildElement("TextLine");
                    textLine != nullptr;
                    textLine = textLine->NextSiblingElement("TextLine")) {

                    int vpos = textLine->IntAttribute("VPOS", -1);

                    // Collect words and their positions
                    struct WordInfo { std::string text; int hpos; int width; };
                    std::vector<WordInfo> words;
                    for (tinyxml2::XMLElement* stringElement = textLine->FirstChildElement("String");
                        stringElement != nullptr;
                        stringElement = stringElement->NextSiblingElement("String")) {
                        const char* content = stringElement->Attribute("CONTENT");
                        int hpos = stringElement->IntAttribute("HPOS", -1);
                        int width = stringElement->IntAttribute("WIDTH", 0);
                        if (content) {
                            words.push_back({ content, hpos, width });
                        }
                    }

                    // Build line text for matching
                    std::string lineText;
                    for (size_t i = 0; i < words.size(); ++i) {
                        if (i > 0) lineText += " ";
                        lineText += words[i].text;
                    }

                    for (const auto& header : headerKeywords) {
                        std::istringstream iss(header);
                        std::vector<std::string> headerWords;
                        std::string word;
                        while (iss >> word) headerWords.push_back(word);

                        for (size_t i = 0; i + headerWords.size() <= words.size(); ++i) {
                            bool match = true;
                            for (size_t j = 0; j < headerWords.size(); ++j) {
                                std::string ocrWord = trim(words[i + j].text);
                                std::string headerWord = trim(headerWords[j]);
                                if (!normalizedEquals(ocrWord, headerWord)) {
                                    match = false;
                                    break;
                                }
                            }

                            if (match) {
                                int xmin = words[i].hpos;
                                int xmax = words[i + headerWords.size() - 1].hpos + words[i + headerWords.size() - 1].width;
                                // Find duplicate headers
                                bool duplicate = false;
                                for (const auto& h : headers) {
                                    if (h.getName() == header && std::abs(h.getXmin() - xmin) < 5 && std::abs(h.getVpos() - vpos) < 5) {
                                        duplicate = true;
                                        break;
                                    }
                                }
                                if (!duplicate) {
                                    Header h(header, xmin, vpos);
                                    h.setXmin(xmin);
                                    h.setXmax(xmax);
                                    headers.push_back(h);                                    
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        return headers;
    }

    // Assign blocks to headers based on their position
    void assignBlocksToHeaders(
        std::vector<Header>& headers,
        std::vector<std::shared_ptr<Block>>& blocks
    ) {
        if (headers.empty()) return;

        // 1. Header nach X-Position sortieren (Angaben zu den Umsätzen immer links)
        std::sort(headers.begin(), headers.end(), [](const Header& a, const Header& b) {
            return a.getXmin() < b.getXmin();
        });

        // Header-Referenzen holen
        Header* descHeader = nullptr;
        Header* valutaHeader = nullptr;
        Header* creditHeader = nullptr;
        Header* debitHeader = nullptr;
        for (auto& h : headers) {
            if (h.getName() == "Angaben zu den Umsätzen") descHeader = &h;
            else if (h.getName() == "Valuta") valutaHeader = &h;
            else if (h.getName() == "zu Ihren Gunsten") creditHeader = &h;
            else if (h.getName() == "zu Ihren Lasten") debitHeader = &h;
        }
        if (!descHeader || !valutaHeader) return;

        int descXmin = descHeader->getXmin();
        int valutaXmin = valutaHeader->getXmin();

        // 2. Blöcke für "Angaben zu den Umsätzen" sammeln und ggf. splitten
        std::vector<Block> descBlocksToMerge;
        std::vector<std::shared_ptr<Block>> blocksForOtherHeaders;

        for (const auto& blockPtr : blocks) {
            int x1 = blockPtr->getX1();
            int x2 = blockPtr->getX2();
            int y1 = blockPtr->getY1();

            // Nur Blöcke unterhalb der Headerzeile
            if (y1 <= descHeader->getVpos()) continue;

            // Block liegt komplett links von Valuta -> zu "Angaben zu den Umsätzen"
            if (x2 <= valutaXmin) {
                descBlocksToMerge.push_back(*blockPtr);
                continue;
            }

            // Block beginnt vor Valuta, reicht aber in Valuta hinein -> splitten
            if (x1 < valutaXmin && x2 > valutaXmin) {
                auto split = blockPtr->splitByXRecursive(valutaXmin);
                for (const auto& part : split) {
                    if (part.getX2() <= valutaXmin)
                        descBlocksToMerge.push_back(part);
                    else
                        blocksForOtherHeaders.push_back(std::make_shared<Block>(part));
                }
                continue;
            }

            // Block liegt komplett rechts von descHeader, also für andere Header
            blocksForOtherHeaders.push_back(blockPtr);
        }

        // 3. Alles links von Valuta mergen und "Angaben zu den Umsätzen" zuordnen
        if (!descBlocksToMerge.empty()) {
            Block merged = Block::mergeBlocks(descBlocksToMerge);
            descHeader->clearBlocks();
            descHeader->addBlock(std::make_shared<Block>(merged));
        }

        // 4. Restliche Blöcke den anderen Headern zuordnen (inkl. Überlappungen)
        for (auto& blockPtr : blocksForOtherHeaders) {
            int x1 = blockPtr->getX1();
            int x2 = blockPtr->getX2();
            int y1 = blockPtr->getY1();

            if (valutaHeader && x2 > valutaHeader->getXmin() && x1 < valutaHeader->getXmax() && y1 > valutaHeader->getVpos())
                valutaHeader->addBlock(blockPtr);
            if (creditHeader && x2 > creditHeader->getXmin() && x1 < creditHeader->getXmax() && y1 > creditHeader->getVpos())
                creditHeader->addBlock(blockPtr);
            if (debitHeader && x2 > debitHeader->getXmin() && x1 < debitHeader->getXmax() && y1 > debitHeader->getVpos())
                debitHeader->addBlock(blockPtr);
        }
    }

    bool fileExists(const std::string& filePath) {
        return std::filesystem::exists(filePath);
    }

    std::string performOCR(const std::string& imageFilePath, const std::string& tessdataPath) {
        tesseract::TessBaseAPI ocr;
        ocr.SetVariable("TESSDATA_PREFIX", tessdataPath.c_str());
        if (ocr.Init(tessdataPath.c_str(), "deu") != 0) {
            throw std::runtime_error("Failed to initialize Tesseract OCR with German language.");
        }
        Pix* image = pixRead(imageFilePath.c_str());
        if (!image) {
            throw std::runtime_error("Failed to read image file: " + imageFilePath);
        }
        ocr.SetImage(image);
        ocr.SetPageSegMode(tesseract::PSM_AUTO);
        std::string extractedText = ocr.GetAltoText(0);
        pixDestroy(&image);
        ocr.End();
        return extractedText;
    }

    std::string convertPdfToImages(const std::string& filePath, const std::string& outputPrefix) {
        std::string command = "pdftoppm -png -r 300 \"" + filePath + "\" " + outputPrefix;
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

    std::vector<std::shared_ptr<TextElement>> parseXmlToTextElements(
        const std::string& xmlContent,
        std::vector<Header>& headers
    ) {
        std::vector<std::shared_ptr<TextElement>> elements;
        tinyxml2::XMLDocument doc;
        if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
            throw std::runtime_error("Failed to parse ALTO XML.");
        }
        tinyxml2::XMLElement* printSpace = doc.FirstChildElement("Page")->FirstChildElement("PrintSpace");
        if (!printSpace) {
            throw std::runtime_error("ALTO XML does not contain a valid PrintSpace element.");
        }

        std::vector<std::string> headerKeywords = {
            "zu Ihren Lasten", "zu Ihren Gunsten", "Angaben zu den Umsätzen", "Valuta"
        };

        headers = findHeaders(printSpace, headerKeywords);

        std::vector<std::shared_ptr<Block>> allBlocks;
        for (tinyxml2::XMLElement* composedBlock = printSpace->FirstChildElement("ComposedBlock");
            composedBlock != nullptr;
            composedBlock = composedBlock->NextSiblingElement("ComposedBlock")) {
            for (tinyxml2::XMLElement* textBlock = composedBlock->FirstChildElement("TextBlock");
                textBlock != nullptr;
                textBlock = textBlock->NextSiblingElement("TextBlock")) {
                try {
                    allBlocks.emplace_back(std::make_shared<Block>(textBlock));
                }
                catch (const std::exception&) {}
            }
        }

        assignBlocksToHeaders(headers, allBlocks);

        for (const auto& h : headers) {
            std::cout << "[DEBUG] Header '" << h.getName() << "' hat " << h.getBlocks().size() << " Blöcke." << std::endl;
        }

        std::vector<Transaction> transactions = extractTransactions(headers);

        for (const auto& block : allBlocks) {
            elements.push_back(block);
        }
        return elements;
    }

} // namespace

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    std::vector<Transaction> allTransactions;
    ConsoleView consoleView;
    if (!fileExists(filePath)) {
        consoleView.displayError("PDF file does not exist: " + filePath);
        throw std::runtime_error("PDF file does not exist: " + filePath);
    }
    const std::string tessdataPath = "C:/coding/fossredder/res/tessdata/";
    const std::string outputPrefix = "page";
    convertPdfToImages(filePath, outputPrefix);
    std::vector<std::shared_ptr<TextElement>> allElements;
    std::vector<Header> allHeaders; // <--- Sammle alle Header
    auto imageFiles = getGeneratedImageFiles(outputPrefix);
    for (const auto& imageFile : imageFiles) {
        try {
            std::string xmlContent = performOCR(imageFile, tessdataPath);
            // Passe parseXmlToTextElements an, sodass sie Header zurückgibt:
            std::vector<Header> headers;
            auto elements = parseXmlToTextElements(xmlContent, headers);
            allElements.insert(allElements.end(), elements.begin(), elements.end());
            allHeaders.insert(allHeaders.end(), headers.begin(), headers.end());
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }
    // Extrahiere Transaktionen aus allen gesammelten Headern
    std::vector<Transaction> transactions = extractTransactions(allHeaders);
    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allElements, transactions);
    return pdfData;
}
