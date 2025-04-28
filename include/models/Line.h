#pragma once
#include "pch.h"
#include "tinyxml2.h"
#include "models/TextElement.h"
#include "models/Word.h"

class Line : public TextElement {
public:
    Line(tinyxml2::XMLElement* element);
    ~Line() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Word> words;

private:
    std::string rawXml;
};

