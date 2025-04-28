#include "models/TextElement.h"

TextElement::TextElement(tinyxml2::XMLElement* element) {
    if (!element) {
        throw std::invalid_argument("Null XML element passed to TextElement constructor.");
    }

    x1 = element->IntAttribute("HPOS", 0);
    y1 = element->IntAttribute("VPOS", 0);
    width = element->IntAttribute("WIDTH", 0);
    height = element->IntAttribute("HEIGHT", 0);
}

TextElement::~TextElement() {}

int TextElement::getX1() const { return x1; }
int TextElement::getY1() const { return y1; }
int TextElement::getWidth() const { return width; }
int TextElement::getHeight() const { return height; }
int TextElement::getX2() const { return x1 + width; }
int TextElement::getY2() const { return y1 + height; }
