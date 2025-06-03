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
    std::vector<BookingGroup> bookingGroups;
    std::vector<std::pair<std::string, int>> bookingDates; // (Datum, Y-Koordinate)

    int relativeYOffset = 0;

    // 1. Finde alle Buchungsdaten
    for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
        const auto& page = pages[pageIndex];
        relativeYOffset = page->getHeight() * pageIndex;

        for (const auto& header : page->getHeaders()) {
            if (header->getName().find("Angaben zu den Umsätzen") != std::string::npos) {
                for (const auto& block : header->getBlocks()) {
                    std::regex bookingDateRegex(R"(Buchungsdatum:\s*(\d{2}\.\d{2}\.\d{4}))");
                    std::smatch match;
                    std::string blockText = block->getFormattedText();

                    if (std::regex_search(blockText, match, bookingDateRegex) && match.size() > 1) {
                        std::string bookingDate = match[1].str();
                        int blockY = block->getY2() + relativeYOffset;
                        bookingDates.push_back({ bookingDate, blockY });
                        std::cout << "Found booking date: " << bookingDate << " at Y: " << blockY << std::endl;
                    }
                }
            }
        }
    }

    // 2. Iteriere über die Buchungsdaten und extrahiere die Blöcke
    for (size_t i = 0; i < bookingDates.size(); ++i) {
        std::string currentBookingDate = bookingDates[i].first;
        int bookingGroupStart = bookingDates[i].second;
        int bookingGroupEnd = (i + 1 < bookingDates.size()) ? bookingDates[i + 1].second : INT_MAX; // Bis zum Ende, wenn es das letzte ist

        std::cout << "Extracting blocks for booking date: " << currentBookingDate
            << " from Y: " << bookingGroupStart << " to Y: " << bookingGroupEnd << std::endl;

        std::vector<std::shared_ptr<Block>> currentDetailsBlocks;
        std::vector<std::shared_ptr<Block>> currentValutaBlocks;
        std::vector<std::shared_ptr<Block>> currentDebitBlocks;
        std::vector<std::shared_ptr<Block>> currentCreditBlocks;

        relativeYOffset = 0;
        for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
            const auto& page = pages[pageIndex];
            relativeYOffset = page->getHeight() * pageIndex;

            for (const auto& header : page->getHeaders()) {
                int headerY = header->getY1() + relativeYOffset;

                if (header->getName().find("Angaben zu den Umsätzen") != std::string::npos) {
                    for (const auto& block : header->getBlocks()) {
                        int blockY = block->getY1() + relativeYOffset;
                        if (blockY >= bookingGroupStart && blockY < bookingGroupEnd) {
                            //Details Blöcke
                            std::regex bookingDateRegex(R"(Buchungsdatum:\s*(\d{2}\.\d{2}\.\d{4}))");
                            std::smatch match;
                            std::string blockText = block->getFormattedText();

                            if (std::regex_search(blockText, match, bookingDateRegex) && match.size() > 1) {
                                int splitY = block->getY2() + relativeYOffset;
                                std::vector<Block> splitBlocks = block->splitAt(TextElement::SplitDirection::HORIZONTAL, splitY);
                                if (splitBlocks.size() > 1) {
                                    currentDetailsBlocks.push_back(std::make_shared<Block>(splitBlocks[1]));
                                }
                            }
                            else {
                                currentDetailsBlocks.push_back(block);
                            }
                        }
                    }
                }
                if (header->getName().find("Valuta") != std::string::npos) {
                    for (const auto& block : header->getBlocks()) {
                        int blockY = block->getY1() + relativeYOffset;
                        if (blockY >= bookingGroupStart && blockY < bookingGroupEnd) {
                            currentValutaBlocks.push_back(block);
                        }
                    }
                }
                if (header->getName().find("zu Ihren Lasten") != std::string::npos) {
                    for (const auto& block : header->getBlocks()) {
                        int blockY = block->getY1() + relativeYOffset;
                        if (blockY >= bookingGroupStart && blockY < bookingGroupEnd) {
                            currentDebitBlocks.push_back(block);
                        }
                    }
                }
                if (header->getName().find("zu Ihren Gunsten") != std::string::npos) {
                    for (const auto& block : header->getBlocks()) {
                        int blockY = block->getY1() + relativeYOffset;
                        if (blockY >= bookingGroupStart && blockY < bookingGroupEnd) {
                            currentCreditBlocks.push_back(block);
                        }
                    }
                }
            }
        }

        bookingGroups.emplace_back(currentBookingDate,
            std::move(currentDetailsBlocks),
            std::move(currentValutaBlocks),
            std::move(currentDebitBlocks),
            std::move(currentCreditBlocks));
    }

    return bookingGroups;
}