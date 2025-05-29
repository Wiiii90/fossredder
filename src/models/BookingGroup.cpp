#include "pch.h"
#include "models/BookingGroup.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"

BookingGroup::BookingGroup(const std::string& bookingDate, std::vector<Transaction>&& transactions)
    : bookingDate(bookingDate), transactions(std::move(transactions)) {
}

const std::string& BookingGroup::getBookingDate() const {
    return bookingDate;
}

const std::vector<Transaction>& BookingGroup::getTransactions() const {
    return transactions;
}

double BookingGroup::parseAmount(const std::string& text) {
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

// Reuse functionality from Header class
std::vector<Transaction> BookingGroup::extractTransactions(const std::vector<Header>& headers) {
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

// Helper function for parsing amounts
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