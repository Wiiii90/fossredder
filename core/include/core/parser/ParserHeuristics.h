#pragma once

#include <string>

namespace core::parser::heuristics {

// Header heuristics
bool isHeaderNoiseLine(const std::string& line);

// Footer and footnote related heuristics
bool isFooterLine(const std::string& line);

bool isPostTransactionFootnote(const std::string& line);

// Transaction-related heuristics (section headers, debit/credit header detection)
bool isTransactionsSectionHeader(const std::string& line);

bool isDebitCreditHeaderLine(const std::string& line);

} // namespace core::parser::heuristics
