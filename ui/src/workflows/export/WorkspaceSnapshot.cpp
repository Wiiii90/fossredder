/**
 * @file ui/src/export/WorkspaceSnapshot.cpp
 * @brief Implementation of the UI workspace snapshot component.
 */

#include "ui/workflows/export/WorkspaceSnapshot.h"

namespace ui::exporting {

std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> createWorkspaceSnapshot(const core::domain::catalog::WorkspaceCatalog& state)
{
    return std::make_shared<core::domain::catalog::WorkspaceCatalog>(state);
}

}

