#include "pch.h"
#include "models/Line.h"

Line::Line(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) {
        throw std::invalid_argument("Null XML element passed to Line constructor.");
    }

    for (tinyxml2::XMLElement* stringElement = element->FirstChildElement("String");
        stringElement != nullptr;
        stringElement = stringElement->NextSiblingElement("String")) {
        words.emplace_back(stringElement);
    }
}

Line::~Line() {}

std::string Line::getRawText() const {
    return rawXml;
}

std::string Line::getFormattedText() const {
    std::string formattedText;
    for (const auto& word : words) {
        formattedText += word.getFormattedText() + " ";
    }
    return formattedText;
}
