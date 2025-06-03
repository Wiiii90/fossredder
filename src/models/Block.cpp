#include "pch.h"
#include "models/Block.h"

Block::Block(tinyxml2::XMLElement* element, Page* page) : TextElement(element, page) {
    for (tinyxml2::XMLElement* paragraphElem = element->FirstChildElement("TextLine");
        paragraphElem != nullptr;
        paragraphElem = paragraphElem->NextSiblingElement("TextLine")) {
        paragraphs.emplace_back(paragraphElem, page);
    }
    if (paragraphs.empty()) {
        paragraphs.emplace_back(element, page);
    }
}

Block::Block(const Block& other)
    : TextElement(other),
    paragraphs(other.paragraphs),
    rawXml(other.rawXml)
{}

Block::~Block() {}

std::string Block::getRawText() const {
    std::string result;
    for (const auto& paragraph : paragraphs) {
        result += paragraph.getRawText() + "\n";
    }
    return result;
}

std::string Block::getFormattedText() const {
    std::string result;
    for (const auto& paragraph : paragraphs) {
        result += paragraph.getFormattedText();
        result += "\n";
    }
    return result;
}

std::vector<Block> Block::splitAt(SplitDirection direction, int coordinate) const {
    std::vector<Paragraph> firstParagraphs, secondParagraphs;
    bool isVertical = (direction == SplitDirection::VERTICAL);

    for (const auto& para : paragraphs) {
        auto splitParas = isVertical ?
            para.splitAt(SplitDirection::VERTICAL, coordinate) :
            para.splitAt(SplitDirection::HORIZONTAL, coordinate);

        if (splitParas.size() == 2) {
            firstParagraphs.push_back(splitParas[0]);
            secondParagraphs.push_back(splitParas[1]);
        }
        else if (splitParas.size() == 1) {
            if (isVertical) {
                if (splitParas[0].getX2() <= coordinate) {
                    firstParagraphs.push_back(splitParas[0]);
                }
                else {
                    secondParagraphs.push_back(splitParas[0]);
                }
            }
            else {
                if (!splitParas[0].lines.empty() && splitParas[0].lines.front().getY1() <= coordinate) {
                    firstParagraphs.push_back(splitParas[0]);
                }
                else {
                    secondParagraphs.push_back(splitParas[0]);
                }
            }
        }
    }

    std::vector<Block> result;
    if (!firstParagraphs.empty()) {
        Block first = *this;
        first.paragraphs = firstParagraphs;
        first.updateBoundingBox();
        result.push_back(first);
    }
    if (!secondParagraphs.empty()) {
        Block second = *this;
        second.paragraphs = secondParagraphs;
        second.updateBoundingBox();
        result.push_back(second);
    }
    return result;
}

void Block::updateBoundingBox() {
    if (paragraphs.empty()) {
        x1 = x2 = y1 = y2 = width = height = 0;
        return;
    }
    x1 = paragraphs.front().getX1();
    x2 = paragraphs.front().getX2();
    y1 = paragraphs.front().getY1();
    y2 = paragraphs.front().getY2();
    for (const auto& p : paragraphs) {
        if (p.getX1() < x1) x1 = p.getX1();
        if (p.getX2() > x2) x2 = p.getX2();
        if (p.getY1() < y1) y1 = p.getY1();
        if (p.getY2() > y2) y2 = p.getY2();
    }
    width = x2 - x1;
    height = y2 - y1;
}

Block Block::merge(const std::vector<Block>& blocks) {
    if (blocks.empty()) throw std::invalid_argument("No blocks to merge.");
    Block merged = blocks.front();
    merged.paragraphs.clear();
    for (const auto& block : blocks) {
        merged.paragraphs.insert(merged.paragraphs.end(), block.paragraphs.begin(), block.paragraphs.end());
    }
    return merged;
}

void Block::setY1(int y1) {
    this->y1 = y1;
}

void Block::setY2(int y2) {
    this->y2 = y2;
}