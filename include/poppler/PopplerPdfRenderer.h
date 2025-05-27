#pragma once
#include "poppler/IPdfRenderer.h"
#include <string>
#include <vector>

class PopplerPdfRenderer : public IPdfRenderer {
public:
    std::vector<std::string> renderToImages(const std::string& pdfPath, const std::string& outputPrefix) override;
};