#pragma once
#include "TextElement.h"

class PdfExtractedData {
public:
    PdfExtractedData(const std::string& sourceFile, const std::vector<std::shared_ptr<TextElement>>& elements);

    const std::string& getSourceFile() const;
    const std::vector<std::shared_ptr<TextElement>>& getElements() const;

private:
    std::string sourceFile;
    std::vector<std::shared_ptr<TextElement>> elements;
};

