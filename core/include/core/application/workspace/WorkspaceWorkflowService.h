/**
 * @file core/include/core/application/workspace/WorkspaceWorkflowService.h
 * @brief Applies draft and log workflow commands to workspace session state.
 */

#pragma once

#include <string>
#include <vector>

#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/ports/workspace/WorkspaceCommands.h"

namespace core::application {

class WorkspaceSession;

class WorkspaceWorkflowService {
public:
    /** @brief Creates workflow service bound to one workspace session. */
    explicit WorkspaceWorkflowService(WorkspaceSession& session);
    ~WorkspaceWorkflowService();

    WorkspaceWorkflowService(const WorkspaceWorkflowService&) = delete;
    WorkspaceWorkflowService& operator=(const WorkspaceWorkflowService&) = delete;
    WorkspaceWorkflowService(WorkspaceWorkflowService&&) noexcept;
    WorkspaceWorkflowService& operator=(WorkspaceWorkflowService&&) noexcept;

    /** @brief Finalizes statement draft and returns created statement id. */
    [[nodiscard]] std::string finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand& command);
    /** @brief Saves or updates statement draft. */
    void saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command);
    /** @brief Clears one statement draft by id, or all drafts when id is empty. */
    void clearStatementDraft(const std::string& draftId = {});

    /** @brief Replaces import logs with command payload. */
    void setImportLogs(const core::ports::workspace::ImportLogsCommand& command);
    /** @brief Replaces export logs with command payload. */
    void setExportLogs(const core::ports::workspace::ExportLogsCommand& command);

private:
    core::application::workspace::WorkspaceSessionState& mutableDocument() noexcept;
    void commit();

    WorkspaceSession* session_ = nullptr;
};

} // namespace core::application
