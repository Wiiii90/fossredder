#include "pch.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"
#include <algorithm>
#include <iostream>
#include <cmath>

Header::Header(const std::string& name, int hpos, int vpos, int xmin, int xmax)
    : name(name), hpos(hpos), vpos(vpos), xmin(xmin), xmax(xmax) {}

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

void Header::assignBlocks(std::vector<Header>& headers, std::vector<std::shared_ptr<Block>>& blocks, int pageWidth) {
    if (headers.empty()) return;

    std::sort(headers.begin(), headers.end(), [](const Header& a, const Header& b) {
        return a.getXmin() < b.getXmin();
    });

    std::vector<int> xCuts;
    for (const auto& h : headers) xCuts.push_back(h.getXmin());
    xCuts.push_back(pageWidth);

    struct HeaderRange {
        Header* header;
        int xMin;
        int xMax;
    };
    std::vector<HeaderRange> headerRanges;
    for (size_t i = 0; i < headers.size(); ++i) {
        int xMin = xCuts[i];
        int xMax = xCuts[i + 1];
        headerRanges.push_back({ &headers[i], xMin, xMax });
        headers[i].clearBlocks();
    }

    for (const auto& blockPtr : blocks) {
        auto splitParts = splitBlockByX(blockPtr, xCuts);
        for (const auto& part : splitParts) {
            int bx1 = part->getX1();
            int bx2 = part->getX2();
            int bcenter = (bx1 + bx2) / 2;
            for (const auto& range : headerRanges) {
                if (bcenter >= range.xMin && bcenter < range.xMax) {
                    range.header->addBlock(part);
                    break;
                }
            }
        }
    }

    for (const auto& h : headers) {
        std::cout << "[DEBUG] Header '" << h.getName() << "' hat " << h.getBlocks().size() << " Blöcke." << std::endl;
        int idx = 0;
        for (const auto& b : h.getBlocks()) {
            std::cout << "  [Block " << idx++ << "] Y1=" << b->getY1() << " X1=" << b->getX1() << " X2=" << b->getX2()
                << " Text: " << b->getFormattedText() << std::endl;
        }
    }
}

void Header::sortBlocks() {
    std::sort(blocks.begin(), blocks.end(), [](const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) {
        if (a->getY1() != b->getY1())
            return a->getY1() < b->getY1();
        return a->getX1() < b->getX1();
        });
}

std::vector<Transaction> Header::extractTransactions(const std::vector<Header>& headers) {
    const Header* valutaHeader = nullptr;
    const Header* descriptionHeader = nullptr;
    const Header* creditHeader = nullptr;
    const Header* debitHeader = nullptr;

    for (const auto& h : headers) {
        std::string n = h.getName();
        if (n == "Valuta") valutaHeader = &h;
        else if (n == "Angaben zu den Umsätzen") descriptionHeader = &h;
        else if (n == "zu Ihren Gunsten") creditHeader = &h;
        else if (n == "zu Ihren Lasten") debitHeader = &h;
    }

    if (!valutaHeader || !descriptionHeader || (!creditHeader && !debitHeader)) {
        std::cout << "[DEBUG] Required headers not found." << std::endl;
        return {};
    }

    const auto& valutaBlocks = valutaHeader->getBlocks();
    if (valutaBlocks.size() < 2) return {};

    std::vector<Transaction> transactions;
    for (size_t i = 1; i < valutaBlocks.size(); ++i) {
        int vpos = valutaBlocks[i]->getY1();
        std::string valuta = valutaBlocks[i]->getFormattedText();
        std::string description, actor, bookingDate;
        double amount = 0.0;
        bool isDebit = false;

        // Find description
        for (const auto& block : descriptionHeader->getBlocks()) {
            if (std::abs(block->getY1() - vpos) < 5) {
                description = block->getFormattedText();
                break;
            }
        }

        // Find amount in credit or debit column
        std::string amountStr;
        if (creditHeader) {
            for (const auto& block : creditHeader->getBlocks()) {
                if (std::abs(block->getY1() - vpos) < 5) {
                    amountStr = block->getFormattedText();
                    isDebit = false;
                    break;
                }
            }
        }
        if (amountStr.empty() && debitHeader) {
            for (const auto& block : debitHeader->getBlocks()) {
                if (std::abs(block->getY1() - vpos) < 5) {
                    amountStr = block->getFormattedText();
                    isDebit = true;
                    break;
                }
            }
        }

        // Parse amount (handle minus sign and German format)
        if (!amountStr.empty()) {
            std::string clean = amountStr;
            clean.erase(std::remove(clean.begin(), clean.end(), '.'), clean.end());
            std::replace(clean.begin(), clean.end(), ',', '.');
            try {
                amount = std::stod(clean);
                if (isDebit && amount > 0) amount = -amount;
            }
            catch (...) {
                std::cout << "[DEBUG] Could not parse amount: " << amountStr << std::endl;
            }
        }

        if (i % 10 == 1) {
            std::cout << "[DEBUG] Transaction: valuta=" << valuta << ", amount=" << amount << ", description=" << description << std::endl;
        }

        transactions.emplace_back(bookingDate, valuta, actor, description, amount, isDebit);
    }
    return transactions;
}