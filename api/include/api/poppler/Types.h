#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace api::poppler {

    struct TextElement {
        std::string text;
        double x = 0.0;
        double y = 0.0;
        double width = 0.0;
        double height = 0.0;
    };

    struct RenderedPage {
        std::string imagePath;
        std::vector<uint8_t> imageBytes;
        int imageWidthPx = 0;
        int imageHeightPx = 0;
        std::string metadataJson;
        double pageWidthPts = 0.0;
        double pageHeightPts = 0.0;
        double dpiX = 0.0, dpiY = 0.0;
        std::vector<TextElement> textElements;
    };

}
