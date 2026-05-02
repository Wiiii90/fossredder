/**
 * @file core/src/import/parsing/DefaultTransactionParser.h
 * @brief Declares the private transaction parser data types and parser entry point.
 */

#pragma once

#include "ParserTypes.h"
#include "AmountParser.h"

#include <string>

namespace core::parser {

struct DefaultTransactionParser {
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string description;
    std::string metadata;
    int status = 0;

    static DefaultTransactionParser parseTransaction(
        const TransactionBlock& block,
        std::vector<std::string>* debugOut = nullptr
    );
};

} // namespace core::parser
