/**
 * @file ui/include/ui/export/WorkspaceSnapshot.h
 * @brief Declarations for the UI workspace snapshot component.
 */

#pragma once

#include <memory>

#include "core/application/workspace/WorkspaceState.h"

namespace ui::exporting {

std::shared_ptr<const core::domain::WorkspaceState> createWorkspaceSnapshot(const core::domain::WorkspaceState& state);

}
