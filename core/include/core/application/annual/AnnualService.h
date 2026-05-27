/**
 * @file core/include/core/application/annual/AnnualService.h
 * @brief Builds annual derived snapshots from workspace snapshots.
 */

#pragma once

#include <string>

#include "core/application/annual/AnnualRequest.h"
#include "core/application/annual/AnnualResult.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::application::annual {

class AnnualService {
public:
    [[nodiscard]] AnnualResult runAnnual(const core::ports::workspace::WorkspaceSnapshot& workspace,
                                         const AnnualRequest& request) const;
    [[nodiscard]] AnnualResult buildAnnualResult(
        const core::ports::workspace::WorkspaceSnapshot& workspace,
        const std::string& annualId) const;
};

} // namespace core::application::annual
