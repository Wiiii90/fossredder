#pragma once

#include <string>
#include <vector>
#include <filesystem>

struct PopplerTextElement {
    std::string text;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct PopplerRenderedPage {
    std::string imagePath;
    std::string metadataJson;
    double pageWidthPts = 0.0;
    double pageHeightPts = 0.0;
    double dpiX = 0.0, dpiY = 0.0;
    std::vector<PopplerTextElement> textElements;
};
