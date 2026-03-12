/**
 * @file core/src/import/parsing/ParserDateUtils.h
 * @brief Declares private helpers for short/full date detection in parser text.
 */

#pragma once

#include <optional>
#include <string>

namespace core::parser::helpers {

bool hasShortDateToken(const std::string& text) noexcept;
bool containsShortDate(const std::string& text) noexcept;
std::optional<std::string> findFirstFullDate(const std::string& text) noexcept;

} // namespace core::parser::helpers
