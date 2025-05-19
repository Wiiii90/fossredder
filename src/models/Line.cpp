#include "pch.h"
#include "models/Line.h"

Line::Line(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Line constructor.");
    for (tinyxml2::XMLElement* stringElem = element->FirstChildElement("String");
        stringElem != nullptr;
        stringElem = stringElem->NextSiblingElement("String")) {
        words.emplace_back(stringElem);
    }
}
Line::~Line() {}

std::string Line::getRawText() const {
    std::string result;
    for (const auto& word : words) {
        result += word.getRawText() + " ";
    }
    return result;
}
std::string Line::getFormattedText() const { return getRawText(); }
