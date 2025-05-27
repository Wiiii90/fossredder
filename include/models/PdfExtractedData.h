#pragma once
#include <string>
#include <vector>
#include <memory>
#include "models/Transaction.h"

class Page;

class PdfExtractedData {
public:
    PdfExtractedData(const std::string& sourceFile,
                     const std::vector<std::shared_ptr<Page>>& pages,
                     const std::vector<Transaction>& transactions);

    const std::string& getSourceFile() const;
    const std::vector<std::shared_ptr<Page>>& getPages() const;
    const std::vector<Transaction>& getTransactions() const;

private:
    std::string sourceFile;
    std::vector<std::shared_ptr<Page>> pages;
    std::vector<Transaction> transactions;
};