#pragma once
#include <string>

namespace tinyxml2 { class XMLElement; }
class Page;

class TextElement {
public:
    TextElement(tinyxml2::XMLElement* element, Page* page);

    TextElement(const TextElement& other);

    virtual ~TextElement();

    enum class SplitDirection {
        HORIZONTAL,
        VERTICAL
    };

    Page* getPage() const;
    void setPage(Page* page);

    int getWidth() const;
    int getHeight() const;
    int getX1() const;
    int getX2() const;
    int getY1() const;
    int getY2() const;

    virtual std::string getRawText() const = 0;
    virtual std::string getFormattedText() const = 0;

protected:
    int x1, y1, x2, y2, width, height;
    Page* page = nullptr;
};