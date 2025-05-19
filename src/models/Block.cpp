#include "pch.h"
#include "models/Block.h"

Block::Block(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Block constructor.");
    for (tinyxml2::XMLElement* paragraphElem = element->FirstChildElement("Paragraph");
        paragraphElem != nullptr;
        paragraphElem = paragraphElem->NextSiblingElement("Paragraph")) {
        paragraphs.emplace_back(paragraphElem);
    }
    // If your ALTO XML does not use <Paragraph>, but only <TextLine>:
    if (paragraphs.empty()) {
        // Fallback: treat the Block as a single Paragraph
        paragraphs.emplace_back(element);
    }
}
Block::~Block() {}

std::string Block::getRawText() const {
    std::string result;
    for (const auto& paragraph : paragraphs) {
        result += paragraph.getRawText() + "\n";
    }
    return result;
}
std::string Block::getFormattedText() const { return getRawText(); }