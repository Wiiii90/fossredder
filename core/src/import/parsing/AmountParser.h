/**
 * @file core/src/import/parsing/AmountParser.h
 * @brief Declares private helpers for parsing OCR amount strings.
 */

#pragma once

#include <optional>
#include <string>

namespace core::parser {

std::optional<double> parseAmountString(const std::string& s);

} // namespace core::parser
