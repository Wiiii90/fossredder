/**
 * @file core/include/core/import/ParserHeuristics.h
 * @brief Declares lightweight text heuristics for statement parsing.
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
