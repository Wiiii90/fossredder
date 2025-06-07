#include "pch.h"
#include "models/TextElement.h"
#include "models/Block.h"
#include "models/Paragraph.h"
#include "models/Line.h"
#include "models/Word.h"
#include "models/Header.h"
#include "tinyxml2.h"

TextElement::TextElement(tinyxml2::XMLElement* element, Page* page) {
    if (!element) {
        throw std::invalid_argument("Null XML element passed to TextElement constructor.");
    }
    if (!page) {
        throw std::invalid_argument("Null Page pointer passed to TextElement constructor.");
    }

    x1 = element->IntAttribute("HPOS", 0);
    y1 = element->IntAttribute("VPOS", 0);
    width = element->IntAttribute("WIDTH", 0);
    height = element->IntAttribute("HEIGHT", 0);
    x2 = x1 + width;
    y2 = y1 + height;
    this->page = page;
}

TextElement::TextElement(const TextElement& other)
    : x1(other.x1), y1(other.y1), x2(other.x2), y2(other.y2),
    width(other.width), height(other.height), page(other.page)
{}

TextElement::~TextElement() {}

Page* TextElement::getPage() const { return page; }

void TextElement::setPage(Page* p) { page = p; }

int TextElement::getWidth() const { return width; }

int TextElement::getHeight() const { return height; }

int TextElement::getX1() const { return x1; }

int TextElement::getX2() const { return x1 + width; }

int TextElement::getY1() const { return y1; }

int TextElement::getY2() const { return y1 + height; }



