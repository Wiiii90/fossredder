#include "pch.h"
#include "controllers/PdfImportController.h"
#include "views/ConsoleView.h"
#include <cstdlib>
#include <windows.h>
#include <cstdio>

int main() {
    SetCurrentDirectoryA("C:\\coding\\fossredder");
    _putenv_s("FONTCONFIG_FILE", "C:\\coding\\fossredder\\res\\fonts.conf");
    _putenv_s("FC_DEBUG", "4");

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    printf("Current working directory: %s\n", cwd);

    PdfImportController pdfController;
    ConsoleView consoleView;

    try {
        auto extractedData = pdfController.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");
        consoleView.displayTransactions(extractedData->getTransactions());
    }
    catch (const std::exception& e) {
        consoleView.displayError("Error during PDF processing: " + std::string(e.what()));
    }

    return 0;
}