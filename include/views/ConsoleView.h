#pragma once
#include <vector>
#include <memory>
#include <string>

class PdfExtractedData;
class Transaction;

class ConsoleView {
public:
    void displayPdfData(const std::shared_ptr<PdfExtractedData>& data);
    void displayError(const std::string& errorMessage);
};