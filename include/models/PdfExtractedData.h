#pragma once
#include "TextElement.h"
#include "Transaction.h"

class PdfExtractedData {
public:
    PdfExtractedData(const std::string& sourceFile,
        const std::vector<std::shared_ptr<TextElement>>& elements,
        const std::vector<Transaction>& transactions);

    const std::string& getSourceFile() const;
    const std::vector<std::shared_ptr<TextElement>>& getElements() const;
    const std::vector<Transaction>& getTransactions() const;

private:
    std::string sourceFile;
    std::vector<std::shared_ptr<TextElement>> elements;
    std::vector<Transaction> transactions;
};