/**
 * @file core/include/core/application/StateProjector.h
 */
#pragma once

#include "core/application/workspace/WorkspaceState.h"

namespace core::application {

class StateProjector {
public:
    static core::domain::WorkspaceState prepareForSave(const core::domain::WorkspaceState& state);
};

}
