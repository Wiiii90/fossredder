#pragma once
#include <string>
#include <vector>
#include <memory>
#include "models/Page.h"
#include "models/Transaction.h"

class BookingGroup {
public:
    BookingGroup(const std::string& bookingDate,
        std::vector<std::shared_ptr<Block>> detailsBlocks = {},
        std::vector<std::shared_ptr<Block>> valutaBlocks = {},
        std::vector<std::shared_ptr<Block>> debitBlocks = {},
        std::vector<std::shared_ptr<Block>> creditBlocks = {});

    const std::string& getBookingDate() const;
    const std::vector<std::shared_ptr<Block>>& getDetailsBlocks() const;
    const std::vector<std::shared_ptr<Block>>& getValutaBlocks() const;
    const std::vector<std::shared_ptr<Block>>& getDebitBlocks() const;
    const std::vector<std::shared_ptr<Block>>& getCreditBlocks() const;

    static std::vector<BookingGroup> extractBookingGroups(const std::vector<std::shared_ptr<Page>>& pages);

private:
    std::string bookingDate;
    std::vector<std::shared_ptr<Block>> detailsBlocks;
    std::vector<std::shared_ptr<Block>> valutaBlocks;
    std::vector<std::shared_ptr<Block>> debitBlocks;
    std::vector<std::shared_ptr<Block>> creditBlocks;
};