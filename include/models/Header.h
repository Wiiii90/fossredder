#pragma once
#include "pch.h"
#include <vector>
#include <memory>

class Block;

class Header {
public:
    Header(const std::string& name, int hpos, int vpos, int xmin = -1, int xmax = -1);

    int getHpos() const;
    int getVpos() const;
    const std::string& getName() const;
    int getXmin() const;
    int getXmax() const;
    void setXmin(int x);
    void setXmax(int x);

    void addBlock(const std::shared_ptr<Block>& block);
    const std::vector<std::shared_ptr<Block>>& getBlocks() const;

    void clearBlocks();

private:
    std::string name;
    int hpos;
    int vpos;
    int xmin;
    int xmax;
    std::vector<std::shared_ptr<Block>> blocks;
};