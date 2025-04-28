#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"
#include "models/PdfExtractedData.h"
#include "tinyxml2.h"
#include <filesystem>
#include "models/Block.h"

PdfImportController::PdfImportController() {}

namespace {
    bool fileExists(const std::string& filePath) {
        std::cout << "[DEBUG] Checking if file exists: " << filePath << std::endl;
        return std::filesystem::exists(filePath);
    }

    std::string performOCR(const std::string& imageFilePath, const std::string& tessdataPath) {
        std::cout << "[DEBUG] Performing OCR on image: " << imageFilePath << std::endl;
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
        std::cout << "[DEBUG] OCR completed for image: " << imageFilePath << std::endl;
        return extractedText;
    }

    std::string convertPdfToImages(const std::string& filePath, const std::string& outputPrefix) {
        std::cout << "[DEBUG] Converting PDF to images: " << filePath << std::endl;
        std::string command = "pdftoppm -png -r 300 \"" + filePath + "\" " + outputPrefix;
        if (std::system(command.c_str()) != 0) {
            throw std::runtime_error("Failed to execute pdftoppm. Ensure it is installed and in your PATH.");
        }
        std::cout << "[DEBUG] PDF conversion to images completed." << std::endl;
        return outputPrefix;
    }

    std::vector<std::string> getGeneratedImageFiles(const std::string& outputPrefix) {
        std::cout << "[DEBUG] Retrieving generated image files with prefix: " << outputPrefix << std::endl;
        std::vector<std::string> imageFiles;
        for (const auto& entry : std::filesystem::directory_iterator(".")) {
            std::string fileName = entry.path().filename().string();
            if (fileName.find(outputPrefix) == 0 && fileName.find(".png") != std::string::npos) {
                imageFiles.push_back(fileName);
                std::cout << "[DEBUG] Found image file: " << fileName << std::endl;
            }
        }
        std::cout << "[DEBUG] Total image files found: " << imageFiles.size() << std::endl;
        return imageFiles;
    }

    std::vector<std::shared_ptr<TextElement>> parseXmlToTextElements(const std::string& xmlContent) {
        std::cout << "[DEBUG] Parsing XML content to TextElements." << std::endl;
        std::cout << "[DEBUG] XML Content: " << xmlContent.substr(0, 500) << "..." << std::endl;

        using namespace tinyxml2;
        std::vector<std::shared_ptr<TextElement>> elements;
        tinyxml2::XMLDocument doc;
        if (doc.Parse(xmlContent.c_str()) != XML_SUCCESS) {
            throw std::runtime_error("Failed to parse ALTO XML.");
        }

        XMLElement* printSpace = doc.FirstChildElement("Page")
            ->FirstChildElement("PrintSpace");
        if (!printSpace) {
            throw std::runtime_error("ALTO XML does not contain a valid PrintSpace element.");
        }

        for (XMLElement* composedBlock = printSpace->FirstChildElement("ComposedBlock");
            composedBlock != nullptr;
            composedBlock = composedBlock->NextSiblingElement("ComposedBlock")) {
            try {
                std::cout << "[DEBUG] Parsing ComposedBlock with attributes: "
                    << "HPOS=" << composedBlock->Attribute("HPOS") << ", "
                    << "VPOS=" << composedBlock->Attribute("VPOS") << ", "
                    << "WIDTH=" << composedBlock->Attribute("WIDTH") << ", "
                    << "HEIGHT=" << composedBlock->Attribute("HEIGHT") << std::endl;

                elements.emplace_back(std::make_shared<Block>(composedBlock));
                std::cout << "[DEBUG] Created Block from XML." << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "[ERROR] Failed to create Block: " << e.what() << std::endl;
            }
        }

        std::cout << "[DEBUG] Total TextElements parsed: " << elements.size() << std::endl;
        return elements;
    }


}

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    ConsoleView consoleView;

    std::cout << "[DEBUG] Starting data extraction for file: " << filePath << std::endl;

    if (!fileExists(filePath)) {
        consoleView.displayError("PDF file does not exist: " + filePath);
        throw std::runtime_error("PDF file does not exist: " + filePath);
    }

    const std::string tessdataPath = "C:/coding/fossredder/res/tessdata/";
    const std::string outputPrefix = "page";

    convertPdfToImages(filePath, outputPrefix);

    std::vector<std::shared_ptr<TextElement>> allElements;
    auto imageFiles = getGeneratedImageFiles(outputPrefix);

    for (const auto& imageFile : imageFiles) {
        try {
            std::cout << "[DEBUG] Processing image file: " << imageFile << std::endl;
            std::string xmlContent = performOCR(imageFile, tessdataPath);
            auto elements = parseXmlToTextElements(xmlContent);
            allElements.insert(allElements.end(), elements.begin(), elements.end());
        }
        catch (const std::exception& e) {
            consoleView.displayError(e.what());
        }
    }

    std::cout << "[DEBUG] Total TextElements extracted: " << allElements.size() << std::endl;

    auto pdfData = std::make_shared<PdfExtractedData>(filePath, allElements);
    std::cout << "[DEBUG] PdfExtractedData object created with " << pdfData->getElements().size() << " elements." << std::endl;

    consoleView.displayPdfData(pdfData);
    std::cout << "[DEBUG] PdfExtractedData displayed in ConsoleView." << std::endl;

    return pdfData;
}


