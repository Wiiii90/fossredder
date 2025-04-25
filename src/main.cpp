#include "pch.h"
#include "controllers/PdfImportController.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // PDF Processing
    PdfImportController pdfController;

    try {
        auto extractedData = pdfController.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");

        // Output the generated image files
        std::cout << "The following image files were generated from the PDF:" << std::endl;
        for (int i = 0; i < extractedData->bookings.size(); ++i) {
            std::cout << "Image file for page " << (i + 1) << ": page_" << (i + 1) << ".png" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error during PDF processing: " << e.what() << std::endl;
    }

    return 0;
}
