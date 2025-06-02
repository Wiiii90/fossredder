#pragma once
#include <vector>
#include <memory>
#include <string>
#include "models/TextElement.h"

class Block;
class Transaction;

class Header : public TextElement {
public:
    Header(tinyxml2::XMLElement* element, Page* page);
    Header(const Header& other);
    ~Header() override;

    void addBlock(const std::shared_ptr<Block>& block);
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    void clearBlocks();
    void sortBlocks();

    const std::string& getName() const;
    std::string getRawText() const override;
    std::string getFormattedText() const override;

    static void sortHeaders(std::vector<Header>& headers);

private:
    std::string name;
    std::vector<std::shared_ptr<Block>> blocks;
};