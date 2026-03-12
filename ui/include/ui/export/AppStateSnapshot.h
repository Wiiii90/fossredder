#pragma once

#include <memory>

#include "core/models/AppState.h"

namespace ui::exporting {

std::shared_ptr<const core::domain::AppState> createSnapshot(const core::domain::AppState& state);

}
