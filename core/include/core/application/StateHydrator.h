/**
 * @file core/include/core/application/StateHydrator.h
 * @brief Rehydration and validation helpers for the in-memory core::domain::AppState.
 */
#pragma once

#include "core/models/AppState.h"

namespace core::application {

class StateHydrator {
public:
    static void rehydrate(core::domain::AppState& state);
    static void validate(const core::domain::AppState& state, bool strictValidation);
};

}
