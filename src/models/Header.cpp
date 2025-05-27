#define NOMINMAX
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

    // 1. Cropping wie gehabt
    int minTableY = std::numeric_limits<int>::max();
    for (const auto& h : headers) {
        if (h.getVpos() < minTableY) minTableY = h.getVpos();
    }
    std::vector<std::shared_ptr<Block>> croppedBlocks;
    for (const auto& b : blocks) {
        if (b->getY1() >= minTableY)
            croppedBlocks.push_back(b);
    }
    int croppedAway = static_cast<int>(blocks.size() - croppedBlocks.size());
    blocks = std::move(croppedBlocks);

    // 2. Tabellenkopf abschneiden wie gehabt
    int maxHeaderY2 = 0;
    for (const auto& h : headers) {
        int y2 = h.getVpos() + 20;
        if (y2 > maxHeaderY2) maxHeaderY2 = y2;
    }
    std::vector<std::shared_ptr<Block>> tableBlocks;
    for (const auto& b : blocks) {
        if (b->getY2() <= maxHeaderY2) continue;
        if (b->getY1() < maxHeaderY2 && b->getY2() > maxHeaderY2) {
            auto splitParts = b->splitByYRecursive(maxHeaderY2);
            for (const auto& part : splitParts) {
                if (part.getY1() > maxHeaderY2)
                    tableBlocks.push_back(std::make_shared<Block>(part));
            }
        }
        else if (b->getY1() >= maxHeaderY2) {
            tableBlocks.push_back(b);
        }
    }
    blocks = std::move(tableBlocks);

    // 3. Header sortieren
    std::sort(headers.begin(), headers.end(), [](const Header& a, const Header& b) {
        return a.getXmin() < b.getXmin();
        });

    // 4. Headerbereiche bestimmen
    struct HeaderRange {
        Header* header;
        int xMin;
        int xMax;
    };
    std::vector<HeaderRange> headerRanges;
    for (size_t i = 0; i < headers.size(); ++i) {
        int xMin = headers[i].getXmin();
        int xMax = (i + 1 < headers.size()) ? headers[i + 1].getXmin() : pageWidth;
        headerRanges.push_back({ &headers[i], xMin, xMax });
        headers[i].clearBlocks();
    }

    // 5. Header-zentrierte, rekursive Zuweisung
    std::vector<std::shared_ptr<Block>> toProcess = blocks;
    for (const auto& range : headerRanges) {
        std::vector<std::shared_ptr<Block>> nextToProcess;
        for (const auto& block : toProcess) {
            int bx1 = block->getX1();
            int bx2 = block->getX2();

            // Block liegt komplett außerhalb des Bereichs: weiterreichen
            if (bx2 <= range.xMin || bx1 >= range.xMax) {
                nextToProcess.push_back(block);
                continue;
            }
            // Block liegt komplett im Bereich: zuordnen
            if (bx1 >= range.xMin && bx2 <= range.xMax) {
                range.header->addBlock(block);
                continue;
            }
            // Block überlappt: splitten an rechter Grenze
            auto splitBlocks = block->splitByXRecursive(range.xMax);
            if (splitBlocks.size() == 1) {
                // Split hat nichts gebracht, trotzdem weiterreichen
                nextToProcess.push_back(std::make_shared<Block>(splitBlocks[0]));
                continue;
            }
            // Linkes Teilstück zuordnen, rechtes weiterreichen
            for (const auto& split : splitBlocks) {
                int sx1 = split.getX1();
                int sx2 = split.getX2();
                if (sx1 >= range.xMin && sx2 <= range.xMax) {
                    range.header->addBlock(std::make_shared<Block>(split));
                }
                else {
                    nextToProcess.push_back(std::make_shared<Block>(split));
                }
            }
        }
        toProcess = std::move(nextToProcess);
    }

    // 6. Debug-Ausgabe
    std::cout << "[DEBUG] " << croppedAway << " Blöcke oberhalb des Tabellenkopfs entfernt." << std::endl;
    if (!toProcess.empty()) {
        std::cout << "[DEBUG] " << toProcess.size() << " Blöcke konnten keinem Header zugeordnet werden." << std::endl;
        int idx = 0;
        for (const auto& b : toProcess) {
            std::cout << "  [Unassigned Block " << idx++ << "] "
                << "Y1=" << b->getY1()
                << " Y2=" << b->getY2()
                << " X1=" << b->getX1()
                << " X2=" << b->getX2()
                << " Text: " << b->getFormattedText() << std::endl;
        }
    }

    for (const auto& h : headers) {
        std::cout << "[DEBUG] Header '" << h.getName()
            << "' Xmin=" << h.getXmin()
            << " Xmax=" << h.getXmax()
            << " Y1=" << h.getVpos()
            << " Y2=" << (h.getVpos() + 20)
            << " hat " << h.getBlocks().size() << " Blöcke." << std::endl;
        int idx = 0;
        for (const auto& b : h.getBlocks()) {
            std::cout << "  [Block " << idx++ << "] Y1=" << b->getY1()
                << " Y2=" << b->getY2()
                << " X1=" << b->getX1()
                << " X2=" << b->getX2()
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