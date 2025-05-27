#pragma once
#include <string>
#include <vector>

class IPdfRenderer {
public:
    virtual ~IPdfRenderer() = default;
    virtual std::vector<std::string> renderToImages(const std::string& pdfPath, const std::string& outputPrefix) = 0;
};