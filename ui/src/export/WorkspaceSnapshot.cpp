/**
 * @file ui/src/export/WorkspaceSnapshot.cpp
 * @brief Implementation of the UI workspace snapshot component.
 */

#include "ui/export/WorkspaceSnapshot.h"

#include "ui/state/WorkspaceClone.h"

namespace ui::exporting {

std::shared_ptr<const WorkspaceState> createWorkspaceSnapshot(const WorkspaceState& state)
{
    return std::make_shared<WorkspaceState>(ui::cloneWorkspaceState(state));
}

}
