#include "pch.h"
#include "models/Paragraph.h"

Paragraph::Paragraph(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) {
        throw std::invalid_argument("Null XML element passed to Paragraph constructor.");
    }

    for (tinyxml2::XMLElement* textLine = element->FirstChildElement("TextLine");
        textLine != nullptr;
        textLine = textLine->NextSiblingElement("TextLine")) {
        lines.emplace_back(textLine);
    }
}

Paragraph::~Paragraph() {}

std::string Paragraph::getRawText() const {
    return rawXml;
}

std::string Paragraph::getFormattedText() const {
    std::string formattedText;
    for (const auto& line : lines) {
        formattedText += line.getFormattedText() + "\n";
    }
    return formattedText;
}
