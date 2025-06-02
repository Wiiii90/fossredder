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
    Paragraph(tinyxml2::XMLElement* element);
    Paragraph(const Paragraph& other);
    ~Paragraph() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Line> lines;

    // New unified splitting method
    std::vector<Paragraph> splitAt(SplitDirection direction, int coordinate) const;

    // Optional: For backward compatibility 
    [[deprecated("Use splitAt(SplitDirection::HORIZONTAL, y) instead")]]
    std::vector<Paragraph> splitByYRecursive(int y) const;

    [[deprecated("Use splitAt(SplitDirection::VERTICAL, x) instead")]]
    std::vector<Paragraph> splitByXRecursive(int x) const;
    
    void updateBoundingBox();

    static Paragraph merge(const std::vector<Paragraph>& paragraphs);

private:
    std::string rawXml;
};