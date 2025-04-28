#include "pch.h"
#include "views/ConsoleView.h"

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    std::cout << "Displaying PDF Data:" << std::endl;
    for (const auto& element : data->getElements()) {
        std::cout << "-----------------------------\n";
        std::cout << "Element:\n";
        std::cout << "Position: (" << element->getX1() << ", " << element->getY1() << ")\n";
        std::cout << "Size: " << element->getWidth() << "x" << element->getHeight() << "\n";
        std::cout << "Text:\n" << element->getFormattedText() << "\n";
        std::cout << "-----------------------------\n";
    }
}

void ConsoleView::displayError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}
