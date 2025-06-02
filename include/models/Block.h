#pragma once
#include <vector>
#include <string>
#include "models/TextElement.h"
#include "models/Page.h"
#include "models/Paragraph.h"

namespace tinyxml2 { class XMLElement; }
class Page;

class Block : public TextElement {
public:
    Block(tinyxml2::XMLElement* element, Page* page);
    Block(const Block& other);
    ~Block() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Paragraph> paragraphs;

    std::vector<Block> splitAt(SplitDirection direction, int coordinate) const;

    void updateBoundingBox();

    static Block merge(const std::vector<Block>& blocks);

private:
    std::string rawXml;
};