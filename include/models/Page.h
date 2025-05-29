#pragma once
#include <vector>
#include <memory>
#include <string>

namespace tinyxml2 { class XMLDocument; class XMLElement; }
class Block;
class Header;

class Page {
public:
    explicit Page(const std::string& altoXml);
    ~Page();

    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    std::vector<Header> extractHeaders(const std::vector<std::string>& headerKeywords) const;

    int getWidth() const;
    int getHeight() const;

private:
    std::vector<std::shared_ptr<Block>> blocks;
    int width = 0;
    int height = 0;
    tinyxml2::XMLDocument* xmlDoc = nullptr;
};