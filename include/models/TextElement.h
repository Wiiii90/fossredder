#pragma once
#include "pch.h"
#include "tinyxml2.h"

class TextElement {
public:
    TextElement(tinyxml2::XMLElement* element);
    virtual ~TextElement();

    int getX1() const;
    int getY1() const;
    int getWidth() const;
    int getHeight() const;
    int getX2() const;
    int getY2() const;

    virtual std::string getRawText() const = 0;
    virtual std::string getFormattedText() const = 0;

protected:
    int x1, y1, width, height;
};
