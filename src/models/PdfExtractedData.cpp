#include "pch.h"
#include "models/PdfExtractedData.h"

PdfExtractedData::PdfExtractedData(const std::string& sourceFile,
    const std::vector<std::shared_ptr<TextElement>>& elements,
    const std::vector<Transaction>& transactions)
    : sourceFile(sourceFile), elements(elements), transactions(transactions)
{
}

const std::string& PdfExtractedData::getSourceFile() const { return sourceFile; }
const std::vector<std::shared_ptr<TextElement>>& PdfExtractedData::getElements() const { return elements; }
const std::vector<Transaction>& PdfExtractedData::getTransactions() const { return transactions; }