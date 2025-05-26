#pragma once
#include <vector>
#include "models/Transaction.h"
#include "models/PdfExtractedData.h"

class ConsoleView {
public:
    void displayPdfData(const std::shared_ptr<PdfExtractedData>& data);
    void displayTransactions(const std::vector<Transaction>& transactions);
    void displayError(const std::string& errorMessage);
};