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
                                    std::cout << "[DEBUG] No match at line VPOS=" << vpos
                                        << " | OCR-Word[" << (i + j) << "]: '" << ocrWord
                                        << "' vs Header-Word[" << j << "]: '" << headerWord << "'"
                                        << std::endl;
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
                                    std::cout << "[DEBUG] Found header: '" << header << "' at xmin=" << xmin << ", xmax=" << xmax << ", vpos=" << vpos << std::endl;
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

    // Assign blocks to headers using header's xmin/xmax and vpos
    void assignBlocksToHeaders(
        const std::vector<Header>& headers,
        std::vector<std::shared_ptr<Block>>& blocks,
        std::unordered_map<std::string, std::vector<std::shared_ptr<Block>>>& headerToBlocks
    ) {
        if (headers.empty()) {
            std::cout << "[DEBUG] No headers found, skipping block assignment." << std::endl;
            return;
        }

        // Find the topmost header (for filtering)
        int minHeaderVpos = std::numeric_limits<int>::max();
        for (const auto& h : headers) minHeaderVpos = std::min(minHeaderVpos, h.getVpos());

        // Remove blocks above the first header
        blocks.erase(std::remove_if(blocks.begin(), blocks.end(),
            [minHeaderVpos](const std::shared_ptr<Block>& block) {
                return block->getY1() < minHeaderVpos;
            }), blocks.end());

        int assigned = 0;
        for (auto& block : blocks) {
            int x1 = block->getX1();
            int x2 = block->getX2();
            int y1 = block->getY1();
            bool found = false;
            for (const auto& h : headers) {
                if (x2 > h.getXmin() && x1 < h.getXmax() && y1 > h.getVpos()) {
                    headerToBlocks[h.getName()].push_back(block);
                    found = true;
                    ++assigned;
                    break;
                }
            }
            if (!found) {
                std::cout << "[DEBUG] Block at (" << x1 << "," << y1 << ") not assigned to any header." << std::endl;
            }
        }
        std::cout << "[DEBUG] Assigned " << assigned << " blocks to headers." << std::endl;
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
        std::unordered_map<std::string, std::vector<std::shared_ptr<Block>>>& headerToBlocks
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

        std::vector<Header> headers = findHeaders(printSpace, headerKeywords);

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

        assignBlocksToHeaders(headers, allBlocks, headerToBlocks);

        for (const auto& block : allBlocks) {
            elements.push_back(block);
        }
        return elements;
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
    std::vector<std::shared_ptr<TextElement>> allElements;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Block>>> headerToBlocks;
    auto imageFiles = getGeneratedImageFiles(outputPrefix);
    for (const auto& imageFile : imageFiles) {
        try {
            std::string xmlContent = performOCR(imageFile, tessdataPath);
            auto elements = parseXmlToTextElements(xmlContent, headerToBlocks);
            allElements.insert(allElements.end(), elements.begin(), elements.end());
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }
    // headerToBlocks enthält jetzt die Zuordnung Header -> Blöcke
    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allElements);
    return pdfData;
}
