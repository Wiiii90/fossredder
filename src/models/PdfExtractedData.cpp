#include "pch.h"
#include "models/PdfExtractedData.h"

PdfExtractedData::PdfExtractedData(const std::string& sourceFile, const std::vector<std::shared_ptr<TextElement>>& elements)
    : sourceFile(sourceFile), elements(elements) {
}

const std::string& PdfExtractedData::getSourceFile() const {
    return sourceFile;
}

const std::vector<std::shared_ptr<TextElement>>& PdfExtractedData::getElements() const {
    return elements;
}

