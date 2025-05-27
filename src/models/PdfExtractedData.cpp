#include "pch.h"
#include "models/PdfExtractedData.h"

PdfExtractedData::PdfExtractedData(const std::string& sourceFile,
    const std::vector<std::shared_ptr<Page>>& pages,
    const std::vector<Transaction>& transactions)
    : sourceFile(sourceFile), pages(pages), transactions(transactions)
{
}

const std::string& PdfExtractedData::getSourceFile() const { return sourceFile; }
const std::vector<std::shared_ptr<Page>>& PdfExtractedData::getPages() const { return pages; }
const std::vector<Transaction>& PdfExtractedData::getTransactions() const { return transactions; }