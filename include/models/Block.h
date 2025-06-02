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
    Block(tinyxml2::XMLElement* element);
    Block(const Block& other);
    ~Block() override;

    std::string getRawText() const override;
    std::string getFormattedText() const override;

    std::vector<Paragraph> paragraphs;

    // Neue vereinheitlichte Methode
    std::vector<Block> splitAt(SplitDirection direction, int coordinate) const;

    // Optional: Für Rückwärtskompatibilität 
    [[deprecated("Use splitAt(SplitDirection::VERTICAL, x) instead")]]
    std::vector<Block> splitByXRecursive(int x) const;

    [[deprecated("Use splitAt(SplitDirection::HORIZONTAL, y) instead")]]
    std::vector<Block> splitByYRecursive(int y) const;

    void updateBoundingBox();

    static Block merge(const std::vector<Block>& blocks);

private:
    std::string rawXml;
};