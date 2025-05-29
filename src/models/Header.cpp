#include "pch.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"
#include "models/Page.h"

Header::Header(tinyxml2::XMLElement* element)
    : TextElement(element) {
    if (element) {
        const char* content = element->Attribute("CONTENT");
        name = content ? content : "";
    }
}

const std::string& Header::getName() const { return name; }

void Header::addBlock(const std::shared_ptr<Block>& block) {
    blocks.push_back(block);
}
const std::vector<std::shared_ptr<Block>>& Header::getBlocks() const {
    return blocks;
}
void Header::clearBlocks() {
    blocks.clear();
}

std::string Header::getRawText() const {
    return name;
}

std::string Header::getFormattedText() const {
    return name;
}

static double parseAmount(const std::string& text) {
    std::string clean = text;
    clean.erase(std::remove(clean.begin(), clean.end(), '.'), clean.end());
    std::replace(clean.begin(), clean.end(), ',', '.');
    try {
        return std::stod(clean);
    }
    catch (...) {
        return 0.0;
    }
}

static std::vector<std::shared_ptr<Block>> splitBlockByX(const std::shared_ptr<Block>& block, const std::vector<int>& xCuts) {
    std::vector<std::shared_ptr<Block>> result;
    std::vector<Block> currentSplits = { *block };
    for (int cut : xCuts) {
        std::vector<Block> newSplits;
        for (const auto& b : currentSplits) {
            auto parts = b.splitByXRecursive(cut);
            for (const auto& part : parts) {
                newSplits.push_back(part);
            }
        }
        currentSplits = newSplits;
    }
    for (const auto& b : currentSplits) {
        result.push_back(std::make_shared<Block>(b));
    }
    return result;
}

void Header::assignBlocks(std::vector<Header>& headers, std::vector<std::shared_ptr<Block>>& blocks) {
    if (headers.empty()) return;

    std::sort(headers.begin(), headers.end(), [](const Header& a, const Header& b) {
        return a.getX1() < b.getX1();
    });

    int pageWidth = 0;
    if (!headers.empty() && headers[0].getPage()) {
        pageWidth = headers[0].getPage()->getWidth();
    }

    for (size_t i = 0; i < headers.size(); ++i) {
        int xMin = headers[i].getX1();
        int xMax = (i + 1 < headers.size()) ? headers[i + 1].getX1() : pageWidth;
        headers[i].clearBlocks();

        std::vector<std::shared_ptr<Block>> nextToProcess;
        for (const auto& block : blocks) {
            int bx1 = block->getX1();
            int bx2 = block->getX2();

            if (bx2 <= xMin || bx1 >= xMax) {
                nextToProcess.push_back(block);
                continue;
            }
            if (bx1 >= xMin && bx2 <= xMax) {
                headers[i].addBlock(block);
                continue;
            }
            auto splitBlocks = block->splitByXRecursive(xMax);
            if (splitBlocks.size() == 1) {
                nextToProcess.push_back(std::make_shared<Block>(splitBlocks[0]));
                continue;
            }
            for (const auto& split : splitBlocks) {
                int sx1 = split.getX1();
                int sx2 = split.getX2();
                if (sx1 >= xMin && sx2 <= xMax) {
                    headers[i].addBlock(std::make_shared<Block>(split));
                }
                else {
                    nextToProcess.push_back(std::make_shared<Block>(split));
                }
            }
        }
        blocks = std::move(nextToProcess);
    }
}

void Header::sortBlocks() {
    std::sort(blocks.begin(), blocks.end(), [](const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) {
        if (a->getY1() != b->getY1())
            return a->getY1() < b->getY1();
        return a->getX1() < b->getX1();
        });
}