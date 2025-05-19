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

PdfImportController::PdfImportController() {}

namespace {

    // Find headers in the XML using the given keywords
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

                    for (tinyxml2::XMLElement* stringElement = textLine->FirstChildElement("String");
                        stringElement != nullptr;
                        stringElement = stringElement->NextSiblingElement("String")) {
                        const char* content = stringElement->Attribute("CONTENT");
                        int hpos = stringElement->IntAttribute("HPOS", -1);
                        if (content) {
                            std::string contentStr(content);
                            for (const auto& header : headerKeywords) {
                                if (contentStr.find(header) != std::string::npos) {
                                    headers.emplace_back(header, hpos, vpos);
                                }
                            }
                        }
                    }
                }
            }
        }
        return headers;
    }

    // Assign blocks to headers based on position, only for blocks after the header row
    void assignBlocksToHeaders(
        const std::vector<Header>& headers,
        std::vector<std::shared_ptr<Block>>& blocks,
        std::unordered_map<std::string, std::vector<std::shared_ptr<Block>>>& headerToBlocks,
        int headerVpos,
        int vposThreshold = 20,
        int hposThreshold = 100
    ) {
        for (auto& block : blocks) {
            if (block->getY1() <= headerVpos + vposThreshold)
                continue;

            int blockHpos = block->getX1();
            int minDist = std::numeric_limits<int>::max();
            std::string assignedHeader;
            for (const auto& header : headers) {
                int dist = std::abs(blockHpos - header.getHpos());
                if (dist < minDist && dist < hposThreshold) {
                    minDist = dist;
                    assignedHeader = header.getName();
                }
            }
            if (!assignedHeader.empty()) {
                headerToBlocks[assignedHeader].push_back(block);
            }
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
            "zu Ihren Lasten", "zu Ihren Gunsten", "Valuta"
        };

        std::vector<Header> headers = findHeaders(printSpace, headerKeywords);

        int headerVpos = std::numeric_limits<int>::max();
        for (const auto& h : headers) {
            headerVpos = std::min(headerVpos, h.getVpos());
        }

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

        assignBlocksToHeaders(headers, allBlocks, headerToBlocks, headerVpos);

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
