#include "pch.h"
#include "models/Word.h"

Word::Word(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
    const char* content = element->Attribute("CONTENT");
    rawXml = content ? content : "";
}

Word::Word(const Word& other)
    : TextElement(other),
    rawXml(other.rawXml)
{
}

Word::~Word() {}

std::string Word::getRawText() const { return rawXml; }
std::string Word::getFormattedText() const { return rawXml; }

std::vector<Word> Word::splitAt(SplitDirection direction, int coordinate) const {
    std::vector<Word> result;
    bool isVertical = (direction == SplitDirection::VERTICAL);

    if (isVertical) {
        if (x1 < coordinate && x2 > coordinate) {
            Word leftPart = *this;
            leftPart.width = coordinate - leftPart.x1;
            leftPart.x2 = coordinate;
            result.push_back(leftPart);

            Word rightPart = *this;
            rightPart.x1 = coordinate;
            rightPart.width = x2 - rightPart.x1;
            result.push_back(rightPart);
        }
        else {
            result.push_back(*this);
        }
    }
    else {
        if (y1 < coordinate && y2 > coordinate) {
            Word upperPart = *this;
            upperPart.height = coordinate - upperPart.y1;
            upperPart.y2 = coordinate;
            result.push_back(upperPart);

            Word lowerPart = *this;
            lowerPart.y1 = coordinate;
            lowerPart.height = y2 - lowerPart.y1;
            result.push_back(lowerPart);
        }
        else {
            result.push_back(*this);
        }
    }
    return result;
}