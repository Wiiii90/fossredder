#pragma once
#include <vector>
#include <memory>
#include <string>
#include "models/Block.h"
#include "models/Header.h"

class Page {
public:
    explicit Page(const std::string& altoXml);

    const std::vector<std::shared_ptr<Block>>& getBlocks() const;
    std::vector<Header> extractHeaders(const std::vector<std::string>& headerKeywords) const;

    int getWidth() const;

private:
    std::vector<std::shared_ptr<Block>> blocks;
    int width = 0;
};