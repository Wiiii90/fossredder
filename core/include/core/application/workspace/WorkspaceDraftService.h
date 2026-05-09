#pragma once

#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/export/ExportLog.h"
#include "core/application/workspace/WorkspaceState.h"

#include <optional>
#include <memory>
#include <string>
#include <vector>

namespace core::application::workspace {

class WorkspaceDraftService {
public:
    static void saveStatementDraft(core::domain::WorkspaceState& state, const core::domain::StatementDraft& draft);
    static void clearStatementDraft(core::domain::WorkspaceState& state, const std::string& draftId = {});
    static std::optional<core::domain::StatementDraft> loadStatementDraft(const core::domain::WorkspaceState& state,
                                                                          const std::string& draftId = {});
};

class WorkspaceLogService {
public:
    static void setImportLogs(core::domain::WorkspaceState& state, const std::vector<core::domain::ImportLog>& logs);
    static std::vector<std::shared_ptr<core::domain::ImportLog>> importLogs(const core::domain::WorkspaceState& state);

    static void setExportLogs(core::domain::WorkspaceState& state, const std::vector<core::domain::ExportLog>& logs);
    static std::vector<std::shared_ptr<core::domain::ExportLog>> exportLogs(const core::domain::WorkspaceState& state);
};

} // namespace core::application::workspace
