#pragma once
#include <vector>
#include <memory>
#include <string>
#include "models/TextElement.h"

class Block;
class Transaction;

class Header : public TextElement {
public:
    Header(tinyxml2::XMLElement* element);

    const std::string& getName() const;

    void addBlock(const std::shared_ptr<Block>& block);
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    void sortBlocks();
    void clearBlocks();

    static void assignBlocks(std::vector<Header>& headers, std::vector<std::shared_ptr<Block>>& blocks);
    static std::vector<Transaction> extractTransactions(const std::vector<Header>& headers);

    std::string getRawText() const override;
    std::string getFormattedText() const override;

private:
    std::string name;
    std::vector<std::shared_ptr<Block>> blocks;
};