#pragma once
#include "pch.h"
#include "models/TextElement.h"
#include "models/Paragraph.h"
#include "tinyxml2.h"
#include <vector>
#include <functional>
#include <utility>

class Block : public TextElement {
public:
    Block(tinyxml2::XMLElement* element);
    ~Block() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Paragraph> paragraphs;

    std::vector<Block> splitAtParagraph(size_t paragraphIdx) const;
    static Block mergeBlocks(const std::vector<Block>& blocks);

    std::pair<Block, Block> splitByY(int y) const;
    std::pair<Block, Block> splitByX(int x) const;
    std::vector<Block> splitByXRecursive(int x) const;
    std::vector<Block> splitByYRecursive(int y) const;

private:
    std::string rawXml;
};