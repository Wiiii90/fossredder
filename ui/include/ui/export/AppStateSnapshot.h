#pragma once

#include <memory>

#include "core/models/AppState.h"

namespace ui::exporting {

std::shared_ptr<const AppState> createSnapshot(const AppState& state);

}
