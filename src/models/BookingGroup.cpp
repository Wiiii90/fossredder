#include "pch.h"
#include "models/BookingGroup.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Transaction.h"

BookingGroup::BookingGroup(const std::string& bookingDate,
    std::vector<std::shared_ptr<Block>> detailsBlocks,
    std::vector<std::shared_ptr<Block>> valutaBlocks,
    std::vector<std::shared_ptr<Block>> debitBlocks,
    std::vector<std::shared_ptr<Block>> creditBlocks)
    : bookingDate(bookingDate),
    detailsBlocks(std::move(detailsBlocks)),
    valutaBlocks(std::move(valutaBlocks)),
    debitBlocks(std::move(debitBlocks)),
    creditBlocks(std::move(creditBlocks)) {
    extractTransactions();
}

const std::string& BookingGroup::getBookingDate() const {
    return bookingDate;
}

const std::vector<std::shared_ptr<Block>>& BookingGroup::getDetailsBlocks() const {
    return detailsBlocks;
}

const std::vector<std::shared_ptr<Block>>& BookingGroup::getValutaBlocks() const {
    return valutaBlocks;
}

const std::vector<std::shared_ptr<Block>>& BookingGroup::getDebitBlocks() const {
    return debitBlocks;
}

const std::vector<std::shared_ptr<Block>>& BookingGroup::getCreditBlocks() const {
    return creditBlocks;
}

std::vector<BookingGroup> BookingGroup::extractBookingGroups(const std::vector<std::shared_ptr<Page>>& pages) {
    struct GroupMark { int pageIdx; int y; std::string date; };
    std::vector<GroupMark> groupMarks;
    for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
        const auto& page = pages[pageIndex];
        for (const auto& header : page->getHeaders()) {
            if (header->getName().find("Angaben zu den Umsätzen") != std::string::npos) {
                for (const auto& block : header->getBlocks()) {
                    std::regex bookingDateRegex(R"(Buchungsdatum:\s*(\d{2}\.\d{2}\.\d{4}))");
                    std::smatch match;
                    std::string blockText = block->getFormattedText();
                    if (std::regex_search(blockText, match, bookingDateRegex) && match.size() > 1) {
                        int splitY = -1;
                        for (const auto& para : block->paragraphs) {
                            for (const auto& line : para.lines) {
                                for (const auto& word : line.words) {
                                    std::string wordText = word.getFormattedText();
                                    if (wordText.find("Buchungsdatum:") != std::string::npos) {
                                        splitY = word.getY2();
                                        break;
                                    }
                                }
                                if (splitY != -1) break;
                            }
                            if (splitY != -1) break;
                        }
                        if (splitY == -1) splitY = block->getY2();
                        groupMarks.push_back({ static_cast<int>(pageIndex), splitY, match[1].str() });
                    }
                }
            }
        }
    }
    std::vector<BookingGroup> bookingGroups;
    for (size_t i = 0; i < groupMarks.size(); ++i) {
        const auto& mark = groupMarks[i];
        int startPage = mark.pageIdx;
        int startY = mark.y;
        int endPage = (i + 1 < groupMarks.size()) ? groupMarks[i + 1].pageIdx : static_cast<int>(pages.size()) - 1;
        int endY = (i + 1 < groupMarks.size()) ? groupMarks[i + 1].y : INT_MAX;
        std::vector<std::shared_ptr<Block>> details, valuta, debit, credit;
        auto blockInRange = [&](const std::shared_ptr<Block>& block, int pageIdx) {
            if (pageIdx == startPage && pageIdx == endPage) {
                return block->getY1() >= startY && block->getY2() <= endY;
            }
            else if (pageIdx == startPage) {
                return block->getY1() >= startY;
            }
            else if (pageIdx == endPage) {
                return block->getY2() <= endY;
            }
            else {
                return true;
            }
            };
        for (int pageIdx = startPage; pageIdx <= endPage; ++pageIdx) {
            const auto& page = pages[pageIdx];
            for (const auto& header : page->getHeaders()) {
                if (header->getName().find("Angaben zu den Umsätzen") != std::string::npos) {
                    for (const auto& block : header->getBlocks()) {
                        std::regex bookingDateRegex(R"(Buchungsdatum:\s*(\d{2}\.\d{2}\.\d{4}))");
                        std::smatch match;
                        std::string blockText = block->getFormattedText();
                        bool isBookingDateBlock = std::regex_search(blockText, match, bookingDateRegex) && match.size() > 1;
                        if (pageIdx == startPage && isBookingDateBlock) {
                            int splitY = -1;
                            for (const auto& para : block->paragraphs) {
                                for (const auto& line : para.lines) {
                                    for (const auto& word : line.words) {
                                        std::string wordText = word.getFormattedText();
                                        if (wordText.find("Buchungsdatum:") != std::string::npos) {
                                            splitY = word.getY2();
                                            break;
                                        }
                                    }
                                    if (splitY != -1) break;
                                }
                                if (splitY != -1) break;
                            }
                            if (splitY == -1) splitY = block->getY2();
                            auto splitBlocks = block->splitAt(TextElement::SplitDirection::HORIZONTAL, splitY);
                            if (splitBlocks.size() > 1) {
                                auto& afterBlock = splitBlocks[1];
                                std::string afterText = afterBlock.getFormattedText();
                                if (!afterText.empty() && !std::regex_search(afterText, bookingDateRegex) && afterBlock.getY2() > afterBlock.getY1()) {
                                    if (blockInRange(std::make_shared<Block>(afterBlock), pageIdx)) {
                                        details.push_back(std::make_shared<Block>(afterBlock));
                                    }
                                }
                            }
                            continue;
                        }
                        if (isBookingDateBlock) {
                            continue;
                        }
                        if (blockInRange(block, pageIdx)) {
                            details.push_back(block);
                        }
                    }
                }
                auto collectBlocks = [&](std::vector<std::shared_ptr<Block>>& target, const std::string& headerName) {
                    if (header->getName().find(headerName) != std::string::npos) {
                        for (const auto& block : header->getBlocks()) {
                            std::regex bookingDateRegex(R"(Buchungsdatum:\s*(\d{2}\.\d{2}\.\d{4}))");
                            std::smatch match;
                            std::string blockText = block->getFormattedText();
                            if (std::regex_search(blockText, match, bookingDateRegex) && match.size() > 1) {
                                continue;
                            }
                            if (blockInRange(block, pageIdx)) {
                                target.push_back(block);
                            }
                        }
                    }
                    };
                collectBlocks(valuta, "Valuta");
                collectBlocks(debit, "zu Ihren Lasten");
                collectBlocks(credit, "zu Ihren Gunsten");
            }
        }
        bookingGroups.emplace_back(mark.date, std::move(details), std::move(valuta), std::move(debit), std::move(credit));
    }
    return bookingGroups;
}

void BookingGroup::extractTransactions() {
    transactions.clear();
    std::vector<std::shared_ptr<Block>> sortedValuta = valutaBlocks;
    std::sort(sortedValuta.begin(), sortedValuta.end(),
        [](const std::shared_ptr<Block>& a, const std::shared_ptr<Block>& b) {
            return a->getY1() < b->getY1();
        });
    for (size_t i = 0; i < sortedValuta.size(); ++i) {
        int startY = sortedValuta[i]->getY1();
        int endY = (i + 1 < sortedValuta.size()) ? sortedValuta[i + 1]->getY1() : INT_MAX;
        std::string valutaDate = sortedValuta[i]->getFormattedText();
        std::string detailsText;
        for (const auto& block : detailsBlocks) {
            if (block->getY1() >= startY && block->getY1() < endY) {
                detailsText += block->getFormattedText();
                detailsText += "\n";
            }
        }
        std::string amountText;
        for (const auto& block : debitBlocks) {
            if (block->getY1() >= startY && block->getY1() < endY) {
                amountText = block->getFormattedText();
                break;
            }
        }
        if (amountText.empty()) {
            for (const auto& block : creditBlocks) {
                if (block->getY1() >= startY && block->getY1() < endY) {
                    amountText = block->getFormattedText();
                    break;
                }
            }
        }
        transactions.emplace_back(bookingDate, valutaDate, detailsText, amountText);
    }
}