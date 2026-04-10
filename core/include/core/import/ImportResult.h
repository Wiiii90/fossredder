/**
 * @file core/include/core/import/ImportResult.h
 * @brief Declares the import execution result contract.
 */

#pragma once

#include "core/models/TransactionDraft.h"

#include <map>
#include <memory>
#include <vector>

namespace core::domain { class Statement; }

struct ImportResult {
    std::shared_ptr<core::domain::Statement> data;
    std::vector<core::domain::TransactionDraft> transactions;
    std::map<std::string, std::vector<uint8_t>> artifacts;
};
