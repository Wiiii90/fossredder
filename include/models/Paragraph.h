#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Line.h"

class Paragraph : public TextElement {
public:
    Paragraph(tinyxml2::XMLElement* element);
    ~Paragraph() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Line> lines;

    std::vector<Paragraph> splitAtLine(size_t lineIdx) const;
    static Paragraph mergeParagraphs(const std::vector<Paragraph>& paragraphs);

    std::pair<Paragraph, Paragraph> splitByY(int y) const;
    std::pair<Paragraph, Paragraph> splitByX(int x) const;
    std::vector<Paragraph> splitByXRecursive(int x) const;
    std::vector<Paragraph> splitByYRecursive(int y) const;

    void updateBoundingBox();

private:
    std::string rawXml;
};