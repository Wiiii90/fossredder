/**
 * @file core/include/core/import/parsing/AmountParser.h
 * @brief Declares the public amount parsing helper used by UI and import code.
 */

#pragma once

#include <optional>
#include <string>

namespace core::parser {

std::optional<double> parseAmountString(const std::string& s);

} // namespace core::parser
