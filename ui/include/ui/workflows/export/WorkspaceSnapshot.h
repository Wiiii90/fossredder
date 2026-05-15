/**
 * @file ui/include/ui/workflows/export/WorkspaceSnapshot.h
 * @brief Declarations for the UI workspace snapshot component.
 */

#pragma once

#include <memory>

#include "core/domain/catalog/WorkspaceCatalog.h"

namespace ui::exporting {

std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> createWorkspaceSnapshot(const core::domain::catalog::WorkspaceCatalog& state);

}

