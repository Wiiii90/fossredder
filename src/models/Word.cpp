#include "pch.h"
#include "models/Word.h"

Word::Word(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
    const char* content = element->Attribute("CONTENT");
    rawXml = content ? content : "";
}

Word::Word(tinyxml2::XMLElement* element) : TextElement(element) {
    const char* content = element->Attribute("CONTENT");
    rawXml = content ? content : "";
}

Word::Word(const Word& other)
    : TextElement(other),
    rawXml(other.rawXml)
{}

Word::~Word() {}

std::string Word::getRawText() const { return rawXml; }
std::string Word::getFormattedText() const { return rawXml; }

std::vector<Word> Word::splitAt(SplitDirection direction, int coordinate) const {
    std::vector<Word> result;
    bool isVertical = (direction == SplitDirection::VERTICAL);
    
    // Words are typically the smallest unit and might not be divisible further
    // Here we implement simple logic to decide which side the word falls on

    if (isVertical) {
        // Vertical splitting (X-coordinate)
        if (getX1() < coordinate && getX2() > coordinate) {
            // Word crosses the boundary, split it
            Word leftPart = *this;
            leftPart.width = coordinate - leftPart.x1;
            result.push_back(leftPart);

            Word rightPart = *this;
            rightPart.x1 = coordinate;
            rightPart.width = rightPart.x2 - rightPart.x1;
            result.push_back(rightPart);
        }
        else {
            // Word doesn't cross the boundary, keep it intact
            result.push_back(*this);
        }
    }
    else {
        // Horizontal splitting (Y-coordinate)
        if (getY1() < coordinate && getY2() > coordinate) {
            // Word crosses the boundary, split it
            Word upperPart = *this;
            upperPart.height = coordinate - upperPart.y1;
            result.push_back(upperPart);

            Word lowerPart = *this;
            lowerPart.y1 = coordinate;
            lowerPart.height = lowerPart.y2 - lowerPart.y1;
            result.push_back(lowerPart);
        }
        else {
            // Word doesn't cross the boundary, keep it intact
            result.push_back(*this);
        }
    }

    return result;
}

// Backward compatibility methods
std::vector<Word> Word::splitByXRecursive(int x) const {
    return splitAt(SplitDirection::VERTICAL, x);
}

std::vector<Word> Word::splitByYRecursive(int y) const {
    return splitAt(SplitDirection::HORIZONTAL, y);
}
