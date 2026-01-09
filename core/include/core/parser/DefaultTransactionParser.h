#pragma once
#include <string>
#include <vector>
#include <optional>
#include <utility>

namespace core::parser {

struct OcrLine {
    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;
    std::vector<std::pair<int, int>> wordSpans;
    std::string text;
};

struct OcrCell {
    OcrLine line;
    bool empty() const { return line.text.empty(); }
};

struct TransactionMainRow {
    OcrCell left;
    OcrCell valuta;
    OcrCell debit;
    OcrCell credit;
};

struct TransactionBlock {
    std::string bookingDateGroup;
    TransactionMainRow main;
    std::vector<OcrLine> detailLines; // left-only
};

struct DefaultTransactionParser {
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string description;
    std::string actorProposal;
    std::string metadata;
    std::string proofImagePath;
    int status = 0;

    static DefaultTransactionParser parseTransaction(const TransactionBlock& block);
};

// Parse an amount token string into a double. Returns std::nullopt if not a valid amount.
std::optional<double> parseAmountString(const std::string& s);

} // namespace core::parser
