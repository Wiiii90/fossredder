#pragma once
#include "pch.h"
#include "models/PdfExtractedData.h"

class ConsoleView {
public:
    void displayPdfData(const std::shared_ptr<PdfExtractedData>& data);
    void displayBlocks(const std::vector<std::string>& blocks, int& globalBlockIndex);
    void displayError(const std::string& errorMessage);
};
