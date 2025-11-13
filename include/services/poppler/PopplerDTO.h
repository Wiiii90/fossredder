#pragma once

#include <string>
#include <vector>

struct PopplerTextElement {
    std::string text;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    std::string font;
    double size = 0.0;
};

struct PopplerRenderedPage {
    std::string imagePath;
    double pageWidthPts = 0.0;
    double pageHeightPts = 0.0;
    double dpiX = 72.0;
    double dpiY = 72.0;
    std::vector<PopplerTextElement> textElements;
    std::string metadataJson;
};
