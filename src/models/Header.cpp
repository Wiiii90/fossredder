#include "pch.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"

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
}

void Header::sortBlocks() {
    std::sort(blocks.begin(), blocks.end(), [](const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) {
        if (a->getY1() != b->getY1())
            return a->getY1() < b->getY1();
        return a->getX1() < b->getX1();
    });
}

std::vector<Transaction> Header::extractTransactions(const std::vector<Header>& headers) {
    // 1. Valuta-Header suchen (dynamisch, z. B. per Substring)
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

    // 2. Alle anderen Header merken
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

        // 3. Für jeden anderen Header: Blöcke im Bereich suchen und mergen
        for (const auto* header : otherHeaders) {
            std::vector<std::shared_ptr<Block>> blocksInRange;
            for (const auto& block : header->getBlocks()) {
                int by1 = block->getY1();
                // Block liegt (teilweise) im Bereich
                if (by1 >= vposStart && by1 < vposEnd) {
                    blocksInRange.push_back(block);
                }
            }
            if (blocksInRange.empty()) continue;

            // Blöcke mergen
            std::vector<Block> blocksToMerge;
            for (const auto& b : blocksInRange) {
                blocksToMerge.push_back(*b);
            }
            if (!blocksToMerge.empty()) {
                Block merged = Block::mergeBlocks(blocksToMerge);
                std::string headerName = header->getName();
                std::string text = merged.getFormattedText();

                // Heuristik: Zuordnung nach Header-Name
                if (headerName.find("Angaben") != std::string::npos) {
                    description = text;
                } else if (headerName.find("Lasten") != std::string::npos) {
                    amount = parseAmount(text);
                    isDebit = true;
                } else if (headerName.find("Gunsten") != std::string::npos) {
                    amount = parseAmount(text);
                    isDebit = false;
                } else if (headerName.find("Buchung") != std::string::npos) {
                    bookingDate = text;
                } else if (headerName.find("Akteur") != std::string::npos) {
                    actor = text;
                }
                // ... ggf. weitere Zuordnungen
            }
        }

        transactions.emplace_back(bookingDate, valuta, actor, description, amount, isDebit);
    }
    return transactions;
}