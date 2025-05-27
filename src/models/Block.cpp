#include "pch.h"
#include "models/Block.h"

Block::Block(tinyxml2::XMLElement* element) : TextElement(element) {
    if (!element) throw std::invalid_argument("Null XML element passed to Block constructor.");
    for (tinyxml2::XMLElement* paragraphElem = element->FirstChildElement("Paragraph");
        paragraphElem != nullptr;
        paragraphElem = paragraphElem->NextSiblingElement("Paragraph")) {
        paragraphs.emplace_back(paragraphElem);
    }
    if (paragraphs.empty()) {
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

std::string Block::getFormattedText() const {
    std::string result;
    for (const auto& para : paragraphs) {
        result += para.getFormattedText();
        result += "\n";
    }
    return result;
}

std::vector<Block> Block::splitAtParagraph(size_t paragraphIdx) const {
    std::vector<Block> result;
    if (paragraphIdx >= paragraphs.size()) return { *this };
    if (paragraphIdx > 0) {
        Block first = *this;
        first.paragraphs = std::vector<Paragraph>(paragraphs.begin(), paragraphs.begin() + paragraphIdx);
        result.push_back(first);
    }
    if (paragraphIdx < paragraphs.size()) {
        Block second = *this;
        second.paragraphs = std::vector<Paragraph>(paragraphs.begin() + paragraphIdx, paragraphs.end());
        result.push_back(second);
    }
    return result;
}

Block Block::mergeBlocks(const std::vector<Block>& blocks) {
    if (blocks.empty()) throw std::invalid_argument("No blocks to merge.");
    Block merged = blocks.front();
    merged.paragraphs.clear();
    for (const auto& block : blocks) {
        merged.paragraphs.insert(merged.paragraphs.end(), block.paragraphs.begin(), block.paragraphs.end());
    }
    return merged;
}

std::pair<Block, Block> Block::splitByY(int y) const {
    Block upper = *this; upper.paragraphs.clear();
    Block lower = *this; lower.paragraphs.clear();
    for (const auto& para : paragraphs) {
        if (para.getY1() <= y)
            upper.paragraphs.push_back(para);
        else
            lower.paragraphs.push_back(para);
    }
    return { upper, lower };
}

std::pair<Block, Block> Block::splitByX(int x) const {
    Block left = *this; left.paragraphs.clear();
    Block right = *this; right.paragraphs.clear();
    for (const auto& para : paragraphs) {
        if (para.getX1() <= x)
            left.paragraphs.push_back(para);
        else
            right.paragraphs.push_back(para);
    }
    return { left, right };
}

std::vector<Block> Block::splitByXRecursive(int x) const {
    std::vector<Paragraph> leftParagraphs, rightParagraphs;
    for (const auto& para : paragraphs) {
        auto splitParas = para.splitByXRecursive(x);
        for (const auto& p : splitParas) {
            if (!p.lines.empty() && p.lines.front().getX1() <= x)
                leftParagraphs.push_back(p);
            else
                rightParagraphs.push_back(p);
        }
    }
    std::vector<Block> result;
    if (!leftParagraphs.empty()) {
        Block left = *this;
        left.paragraphs = leftParagraphs;
        result.push_back(left);
    }
    if (!rightParagraphs.empty()) {
        Block right = *this;
        right.paragraphs = rightParagraphs;
        result.push_back(right);
    }
    return result;
}

std::vector<Block> Block::splitByYRecursive(int y) const {
    std::vector<Paragraph> upperParagraphs, lowerParagraphs;
    for (const auto& para : paragraphs) {
        auto splitParas = para.splitByYRecursive(y);
        for (const auto& p : splitParas) {
            if (!p.lines.empty() && p.lines.front().getY1() <= y)
                upperParagraphs.push_back(p);
            else
                lowerParagraphs.push_back(p);
        }
    }
    std::vector<Block> result;
    if (!upperParagraphs.empty()) {
        Block upper = *this;
        upper.paragraphs = upperParagraphs;
        result.push_back(upper);
    }
    if (!lowerParagraphs.empty()) {
        Block lower = *this;
        lower.paragraphs = lowerParagraphs;
        result.push_back(lower);
    }
    return result;
}