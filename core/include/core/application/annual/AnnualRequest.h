/**
 * @file core/include/core/application/annual/AnnualRequest.h
 * @brief Declares the annual query request contract.
 */

#pragma once

#include <string>

namespace core::application::annual {

struct AnnualRequest {
    std::string annualId;

    [[nodiscard]] bool empty() const noexcept {
        return annualId.empty();
    }
};

} // namespace core::application::annual

