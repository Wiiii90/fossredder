/**
 * @file core/src/import/parsing/ParserHeuristics.h
 * @brief Declares private text heuristics shared within the statement parser implementation.
 */

#pragma once

#include <string>

namespace core::parser::heuristics {

bool isHeaderNoiseLine(const std::string& line);
bool isFooterLine(const std::string& line);
bool isPostTransactionFootnote(const std::string& line);
bool isTransactionsSectionHeader(const std::string& line);
bool isDebitCreditHeaderLine(const std::string& line);

} // namespace core::parser::heuristics
