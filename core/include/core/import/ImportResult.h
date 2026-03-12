/**
 * @file core/include/core/import/ImportResult.h
 * @brief Declares the import execution result contract.
 */

#pragma once

#include "core/import/ImportedTransaction.h"

#include <map>
#include <memory>
#include <vector>

namespace core::domain { class Statement; }

struct ImportResult {
    std::shared_ptr<core::domain::Statement> data;
    std::vector<ImportedTransaction> transactions;
    std::map<std::string, std::vector<uint8_t>> artifacts;
};
