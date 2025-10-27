#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
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

    const std::vector<Transaction>& getTransactions() const { return transactions; }

    void extractTransactions();

    // Apply a transformation to each transaction's details (e.g., ONNX text cleaner)
    void transformTransactionDetails(const std::function<std::string(const std::string&)>& transform);

private:
    std::string bookingDate;
    std::vector<std::shared_ptr<Block>> detailsBlocks;
    std::vector<std::shared_ptr<Block>> valutaBlocks;
    std::vector<std::shared_ptr<Block>> debitBlocks;
    std::vector<std::shared_ptr<Block>> creditBlocks;

    std::vector<Transaction> transactions;
};