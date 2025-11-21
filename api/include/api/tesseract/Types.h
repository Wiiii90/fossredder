#pragma once

#include <string>

namespace api::tesseract {

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct Word {
    Rect bbox;
    std::string text;
    int confidence = 0;
};

}
