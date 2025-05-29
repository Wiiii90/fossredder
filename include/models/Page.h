#pragma once
#include <vector>
#include <memory>
#include <string>

namespace tinyxml2 { class XMLDocument; class XMLElement; }
class Block;
class Header;

class Page {
public:
    // Crop direction enum for specifying which parts to remove
    enum class CropDirection {
        LEFT,    // Remove blocks to the left of X coordinate
        RIGHT,   // Remove blocks to the right of X coordinate
        TOP,     // Remove blocks above Y coordinate
        BOTTOM   // Remove blocks below Y coordinate
    };
    
    explicit Page(const std::string& altoXml, int index);
    ~Page();

    // Existing methods
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    std::vector<Header> extractHeaders(const std::vector<std::string>& headerKeywords) const;

    // Simplified crop function that does both splitting and removing in one operation
    void crop(CropDirection direction, int boundary);
    
    // Utility method to split blocks at a coordinate
    void splitBlocksAt(bool horizontal, int coordinate);

    int getWidth() const;
    int getHeight() const;
    int getIndex() const;

private:
    std::vector<std::shared_ptr<Block>> blocks;
    int width = 0;
    int height = 0;
    int index = 0;
    tinyxml2::XMLDocument* xmlDoc = nullptr;
};