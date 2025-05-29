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

    int minTableY = std::numeric_limits<int>::max();
    for (const auto& h : headers) {
        if (h.getY1() < minTableY) minTableY = h.getY1();
    }
    std::vector<std::shared_ptr<Block>> croppedBlocks;
    for (const auto& b : blocks) {
        if (b->getY1() >= minTableY)
            croppedBlocks.push_back(b);
    }
    blocks = std::move(croppedBlocks);

    int maxHeaderY2 = 0;
    for (const auto& h : headers) {
        int y2 = h.getY1() + 20;
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

    // Sortiere Header mit vorhandener Funktion
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

std::vector<Transaction> Header::extractTransactions(const std::vector<Header>& headers) {
    const Header* valutaHeader = nullptr;
    for (const auto& h : headers) {
        if (h.getName().find("Valuta") != std::string::npos) {
            valutaHeader = &h;
            break;
        }
    }
    if (!valutaHeader || valutaHeader->getBlocks().size() < 2) {
        std::cout << "[DEBUG] Kein Valuta-Header oder zu wenige Einträge gefunden.\n";
        return {};
    }

    std::vector<const Header*> otherHeaders;
    for (const auto& h : headers) {
        if (&h != valutaHeader)
            otherHeaders.push_back(&h);
    }

    const auto& valutaBlocks = valutaHeader->getBlocks();
    std::vector<Transaction> transactions;

    for (size_t i = 1; i < valutaBlocks.size(); ++i) {
        int vposStart = valutaBlocks[i]->getY1();
        int vposEnd = (i + 1 < valutaBlocks.size()) ? valutaBlocks[i + 1]->getY1() : std::numeric_limits<int>::max();

        std::string valuta = valutaBlocks[i]->getFormattedText();
        std::string description, actor, bookingDate;
        double amount = 0.0;
        bool isDebit = false;

        for (const auto* header : otherHeaders) {
            std::vector<std::shared_ptr<Block>> blocksInRange;
            for (const auto& block : header->getBlocks()) {
                int by1 = block->getY1();
                if (by1 >= vposStart && by1 < vposEnd) {
                    blocksInRange.push_back(block);
                }
            }
            if (blocksInRange.empty()) continue;

            std::vector<Block> blocksToMerge;
            for (const auto& b : blocksInRange) {
                blocksToMerge.push_back(*b);
            }
            if (!blocksToMerge.empty()) {
                Block merged = Block::mergeBlocks(blocksToMerge);
                std::string headerName = header->getName();
                std::string text = merged.getFormattedText();

                if (headerName.find("Angaben") != std::string::npos) {
                    description = text;
                }
                else if (headerName.find("Lasten") != std::string::npos) {
                    amount = parseAmount(text);
                    isDebit = true;
                }
                else if (headerName.find("Gunsten") != std::string::npos) {
                    amount = parseAmount(text);
                    isDebit = false;
                }
                else if (headerName.find("Buchung") != std::string::npos) {
                    bookingDate = text;
                }
                else if (headerName.find("Akteur") != std::string::npos) {
                    actor = text;
                }
            }
        }

        transactions.emplace_back(bookingDate, valuta, actor, description, amount, isDebit);
    }
    return transactions;
}