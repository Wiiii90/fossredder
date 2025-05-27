#include "pch.h"
#include "controllers/PdfImportController.h"
#include "ocr/TesseractOcrEngine.h"
#include "views/ConsoleView.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    auto ocrEngine = std::make_shared<TesseractOcrEngine>();
    PdfImportController controller(ocrEngine);
    ConsoleView consoleView;

    try {
        auto extractedData = controller.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");
        consoleView.displayTransactions(extractedData->getTransactions());
    }
    catch (const std::exception& e) {
        consoleView.displayError("Error during PDF processing: " + std::string(e.what()));
    }

    return 0;
}