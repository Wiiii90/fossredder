#pragma once

#include "core/models/AppState.h"

namespace core::application {

class StateHydrator {
public:
    static void rehydrate(AppState& state);
    static void validate(const AppState& state, bool strictValidation);
};

}
