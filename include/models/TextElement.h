#pragma once
#include <string>

namespace tinyxml2 { class XMLElement; }
class Page;

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

    Page* getPage() const;
    void setPage(Page* page);

    virtual std::string getRawText() const = 0;
    virtual std::string getFormattedText() const = 0;

protected:
    int x1, y1, x2, y2, width, height;
    Page* page = nullptr;
};