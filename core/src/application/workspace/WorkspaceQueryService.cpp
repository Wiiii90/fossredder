/**
 * @file core/src/application/workspace/WorkspaceQueryService.cpp
 * @brief Implements snapshot-based workspace read-side query projections.
 */

#include "core/pch.h"

#include "core/application/workspace/WorkspaceQueryService.h"

#include "core/application/workspace/WorkspaceSession.h"

namespace core::application {

WorkspaceQueryService::WorkspaceQueryService(WorkspaceSession& session)
    : session_(&session) {
}

WorkspaceQueryService::WorkspaceQueryService(WorkspaceQueryService&&) noexcept = default;

WorkspaceQueryService& WorkspaceQueryService::operator=(WorkspaceQueryService&&) noexcept = default;

const core::application::workspace::WorkspaceSessionState& WorkspaceQueryService::stateRef() const noexcept {
    return session_->state();
}

core::ports::workspace::WorkspaceSnapshot WorkspaceQueryService::workspaceSnapshot() const {
    return projector_.project(stateRef(), currentPath());
}

const std::string& WorkspaceQueryService::currentPath() const noexcept {
    return session_->currentPath();
}

std::optional<core::ports::workspace::StatementDraftSnapshot> WorkspaceQueryService::statementDraftSnapshot(const std::string& draftId) const {
    return projector_.projectStatementDraft(stateRef(), draftId);
}

} // namespace core::application
