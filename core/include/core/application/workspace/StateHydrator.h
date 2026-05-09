/**
 * @file core/include/core/application/StateHydrator.h
 * @brief Rehydration and validation helpers for the in-memory core::domain::WorkspaceState.
 */
#pragma once

#include "core/application/workspace/WorkspaceState.h"

namespace core::application {

class StateHydrator {
public:
    static void rehydrate(core::domain::WorkspaceState& state);
    static void validate(const core::domain::WorkspaceState& state, bool strictValidation);
};

}
