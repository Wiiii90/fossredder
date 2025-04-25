#include "pch.h"
#include "controllers/PdfImportController.h"
#include "models/PdfExtractedData.h"

namespace fs = std::filesystem;

// Helper function to check if a file exists
bool fileExists(const std::string& filePath) {
    return fs::exists(filePath);
}

std::shared_ptr<PdfExtractedData> PdfImportController::extractData(const std::string& filePath) {
    // Check if the input PDF file exists
    if (!fileExists(filePath)) {
        throw std::runtime_error("PDF file does not exist: " + filePath);
    }

    // Create a PdfExtractedData object
    auto data = std::make_shared<PdfExtractedData>(filePath);

    // Define the output prefix for the generated image files
    std::string outputPrefix = "page_";

    // Construct the pdftoppm command
    std::string command = "pdftoppm -png -r 300 \"" + filePath + "\" " + outputPrefix;

    // Execute the pdftoppm command
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to execute pdftoppm. Ensure it is installed and in your PATH.");
    }

    // Iterate over the generated image files
    int pageIndex = 1;
    while (true) {
        // Construct the expected file name for the current page
        std::string imageFilePath = outputPrefix + std::to_string(pageIndex) + ".png";

        // Check if the file exists
        if (!fileExists(imageFilePath)) {
            break; // No more pages to process
        }

        // Log the generated image file
        std::cout << "Generated image file: " << imageFilePath << std::endl;

        // Placeholder for further processing (e.g., OCR or adding to PdfExtractedData)
        // You can extend this logic as needed

        ++pageIndex;
    }

    return data;
}
