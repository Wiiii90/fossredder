#pragma once

#include "core/models/AppState.h"

namespace core::application {

class StateProjector {
public:
    static AppState prepareForSave(const AppState& state);
};

}
