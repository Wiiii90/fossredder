#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Paragraph.h"

namespace tinyxml2 { class XMLElement; }

class Block : public TextElement {
public:
    Block(tinyxml2::XMLElement* element);
    Block(const Block& other);
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

    void updateBoundingBox();

private:
    std::string rawXml;
};