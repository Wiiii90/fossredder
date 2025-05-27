#pragma once
#include "pch.h"
#include "models/Page.h"
#include "models/Transaction.h"
#include <memory>
#include <vector>
#include <string>

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