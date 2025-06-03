#include "pch.h"
#include "models/Word.h"

Word::Word(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
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

    std::cout << "Splitting Word at coordinate: " << coordinate << ", direction: " << (isVertical ? "VERTICAL" : "HORIZONTAL") << std::endl;
    std::cout << "Word Y1: " << y1 << ", Word Y2: " << y2 << std::endl;
    std::cout << "Word X1: " << x1 << ", Word X2: " << x2 << std::endl;

    if (isVertical) {
        // Vertical splitting (X-coordinate)
        if (x1 < coordinate && x2 > coordinate) {
            std::cout << "Word crosses vertical boundary" << std::endl;
            // Word crosses the boundary, split it
            Word leftPart = *this;
            leftPart.width = coordinate - leftPart.x1;
            leftPart.x2 = coordinate;
            std::cout << "Left Part X1: " << leftPart.x1 << ", X2: " << leftPart.x2 << ", Width: " << leftPart.width << std::endl;
            result.push_back(leftPart);

            Word rightPart = *this;
            rightPart.x1 = coordinate;
            rightPart.width = x2 - rightPart.x1;
            std::cout << "Right Part X1: " << rightPart.x1 << ", X2: " << rightPart.x2 << ", Width: " << rightPart.width << std::endl;
            result.push_back(rightPart);
        }
        else {
            // Word doesn't cross the boundary, keep it intact
            std::cout << "Word does not cross vertical boundary" << std::endl;
            result.push_back(*this);
        }
    }
    else {
        // Horizontal splitting (Y-coordinate)
        if (y1 < coordinate && y2 > coordinate) {
            std::cout << "Word crosses horizontal boundary" << std::endl;
            // Word crosses the boundary, split it
            Word upperPart = *this;
            upperPart.height = coordinate - upperPart.y1;
            upperPart.y2 = coordinate;
            std::cout << "Upper Part Y1: " << upperPart.y1 << ", Y2: " << upperPart.y2 << ", Height: " << upperPart.height << std::endl;
            result.push_back(upperPart);

            Word lowerPart = *this;
            lowerPart.y1 = coordinate;
            lowerPart.height = y2 - lowerPart.y1;
            std::cout << "Lower Part Y1: " << lowerPart.y1 << ", Y2: " << lowerPart.y2 << ", Height: " << lowerPart.height << std::endl;
            result.push_back(lowerPart);
        }
        else {
            // Word doesn't cross the boundary, keep it intact
            std::cout << "Word does not cross horizontal boundary" << std::endl;
            result.push_back(*this);
        }
    }

    std::cout << "Returning " << result.size() << " words" << std::endl;
    return result;
}