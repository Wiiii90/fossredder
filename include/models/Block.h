#pragma once
#include "pch.h"
#include "models/TextElement.h"
#include "models/Paragraph.h"
#include "tinyxml2.h"

class Block : public TextElement {
public:
    Block(tinyxml2::XMLElement* element);
    ~Block() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Paragraph> paragraphs;

private:
    std::string rawXml;
};

