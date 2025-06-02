#pragma once
#include <vector>
#include <memory>
#include <string>

namespace tinyxml2 { class XMLDocument; class XMLElement; }
class Block;
class Header;

class Page {
public:
    enum class CropDirection {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };

    Page(const std::string& altoXml, int index,
        const std::vector<std::string>& headerKeywords = {},
        const std::vector<std::string>& footerKeywords = {});

    ~Page();

    int getIndex() const;
    int getWidth() const;
    int getHeight() const;
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    const std::vector<std::shared_ptr<Header>>& getHeaders() const;

    void crop(CropDirection direction, int boundary);

private:
    int index = 0;
    int width = 0;
    int height = 0;
    tinyxml2::XMLDocument* xmlDoc = nullptr;
    std::vector<std::shared_ptr<Block>> blocks;
    std::vector<std::shared_ptr<Header>> headers;

    void extractTextElements(const std::string& altoXml);
    void extractHeaders(const std::vector<std::string>& headerKeywords);
    void associateBlocksWithHeaders();
};