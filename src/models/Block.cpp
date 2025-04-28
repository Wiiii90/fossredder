#include "pch.h"
#include "models/Block.h"

Block::Block(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) {
        throw std::invalid_argument("Null XML element passed to Block constructor.");
    }

    for (tinyxml2::XMLElement* textBlock = element->FirstChildElement("TextBlock");
        textBlock != nullptr;
        textBlock = textBlock->NextSiblingElement("TextBlock")) {
        paragraphs.emplace_back(textBlock);
    }
}

Block::~Block() {}

std::string Block::getRawText() const {
    return rawXml;
}

std::string Block::getFormattedText() const {
    std::string formattedText;
    for (const auto& paragraph : paragraphs) {
        formattedText += paragraph.getFormattedText() + "\n";
    }
    return formattedText;
}
