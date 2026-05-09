/**
 * @file core/include/core/application/import/transaction/AmountParser.h
 * @brief Declares helpers for parsing transaction amount strings.
 */

#pragma once

#include <optional>
#include <string>

namespace core::application::importing::transaction {

/**
 * @brief Parses a free-form amount string into a numeric value.
 * @param s Input amount text.
 * @return Parsed amount when the string can be interpreted, otherwise an empty optional.
 */
std::optional<double> parseAmountString(const std::string& s);

} // namespace core::application::importing::transaction

namespace core {
namespace parser {
using application::importing::transaction::parseAmountString;
} // namespace parser
} // namespace core
