#pragma once
#include "pch.h"
#include "models/TextElement.h"
#include "tinyxml2.h"

class Word : public TextElement {
public:
    Word(tinyxml2::XMLElement* element);
    ~Word() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

private:
    std::string rawXml;
};

