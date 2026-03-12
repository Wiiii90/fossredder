/**
 * @file core/include/core/application/StateProjector.h
 */
#pragma once

#include "core/models/AppState.h"

namespace core::application {

class StateProjector {
public:
    static core::domain::AppState prepareForSave(const core::domain::AppState& state);
};

}
