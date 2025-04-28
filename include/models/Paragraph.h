#pragma once
#include "pch.h"
#include "tinyxml2.h"
#include "models/TextElement.h"
#include "models/Line.h"

class Paragraph : public TextElement {
public:
    Paragraph(tinyxml2::XMLElement* element);
    ~Paragraph() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Line> lines;

private:
    std::string rawXml;
};

