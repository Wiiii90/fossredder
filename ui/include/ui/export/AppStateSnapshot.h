/**
 * @file ui/include/ui/export/AppStateSnapshot.h
 * @brief Declarations for the UI AppStateSnapshot component.
 */

#pragma once

#include <memory>

#include "core/models/AppState.h"

namespace ui::exporting {

std::shared_ptr<const core::domain::AppState> createSnapshot(const core::domain::AppState& state);

}
