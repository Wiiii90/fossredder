#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Page.h"
#include "models/Line.h"

namespace tinyxml2 { class XMLElement; }

class Paragraph : public TextElement {
public:
    Paragraph(tinyxml2::XMLElement* element, Page* page);
    Paragraph(const Paragraph& other);
    ~Paragraph() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Line> lines;

    std::vector<Paragraph> splitAt(SplitDirection direction, int coordinate) const;
    
    void updateBoundingBox();

    static Paragraph merge(const std::vector<Paragraph>& paragraphs);

private:
    std::string rawXml;
};