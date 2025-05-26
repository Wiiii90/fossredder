#include "pch.h"
#include "models/Header.h"
#include "models/Block.h"

Header::Header(const std::string& name, int hpos, int vpos, int xmin, int xmax)
    : name(name), hpos(hpos), vpos(vpos), xmin(xmin), xmax(xmax) {
}

int Header::getHpos() const { return hpos; }
int Header::getVpos() const { return vpos; }
const std::string& Header::getName() const { return name; }
int Header::getXmin() const { return xmin; }
int Header::getXmax() const { return xmax; }
void Header::setXmin(int x) { xmin = x; }
void Header::setXmax(int x) { xmax = x; }

void Header::addBlock(const std::shared_ptr<Block>& block) {
    blocks.push_back(block);
}
const std::vector<std::shared_ptr<Block>>& Header::getBlocks() const {
    return blocks;
}

void Header::clearBlocks() {
    blocks.clear();
}