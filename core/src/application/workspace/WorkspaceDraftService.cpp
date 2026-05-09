#include "core/application/workspace/WorkspaceDraftService.h"

#include "../../utils/StableId.h"

#include <algorithm>

namespace core::application::workspace {

void WorkspaceDraftService::saveStatementDraft(core::domain::WorkspaceState& state, const core::domain::StatementDraft& draft)
{
    auto statementDraft = std::make_shared<core::domain::StatementDraft>(draft);
    if (!statementDraft) {
        return;
    }

    if (statementDraft->id.empty()) {
        statementDraft->id = core::utils::makeStableId();
    }

    auto statementIt = std::find_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                                    [statementDraft](const auto& item) {
                                        return item && item->id == statementDraft->id;
                                    });
    if (statementIt == state.statementDrafts.end()) {
        state.statementDrafts.push_back(statementDraft);
    } else {
        *statementIt = statementDraft;
    }

    state.transactionDrafts.erase(
        std::remove_if(state.transactionDrafts.begin(), state.transactionDrafts.end(),
                       [statementDraft](const auto& tx) {
                           return tx && tx->statementDraftId == statementDraft->id;
                       }),
        state.transactionDrafts.end());

    for (std::size_t i = 0; i < statementDraft->transactions.size(); ++i) {
        auto tx = std::make_shared<core::domain::TransactionDraft>(statementDraft->transactions[i]);
        if (!tx) continue;
        if (tx->id.empty()) {
            tx->id = core::utils::makeStableId();
        }
        tx->statementDraftId = statementDraft->id;
        tx->position = static_cast<int>(i);
        state.transactionDrafts.push_back(std::move(tx));
    }
}

void WorkspaceDraftService::clearStatementDraft(core::domain::WorkspaceState& state, const std::string& draftId)
{
    if (draftId.empty()) {
        state.statementDrafts.clear();
        state.transactionDrafts.clear();
        return;
    }

    state.statementDrafts.erase(
        std::remove_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                       [&draftId](const auto& draft) {
                           return draft && draft->id == draftId;
                       }),
        state.statementDrafts.end());

    state.transactionDrafts.erase(
        std::remove_if(state.transactionDrafts.begin(), state.transactionDrafts.end(),
                       [&draftId](const auto& tx) {
                           return tx && tx->statementDraftId == draftId;
                       }),
        state.transactionDrafts.end());
}

std::optional<core::domain::StatementDraft> WorkspaceDraftService::loadStatementDraft(const core::domain::WorkspaceState& state,
                                                                                       const std::string& draftId)
{
    if (state.statementDrafts.empty()) {
        return std::nullopt;
    }

    auto statementIt = state.statementDrafts.begin();
    if (!draftId.empty()) {
        statementIt = std::find_if(state.statementDrafts.begin(), state.statementDrafts.end(),
                                   [&draftId](const auto& draft) {
                                       return draft && draft->id == draftId;
                                   });
    }
    if (statementIt == state.statementDrafts.end() || !*statementIt) {
        return std::nullopt;
    }

    core::domain::StatementDraft out = **statementIt;
    out.transactions.clear();

    std::vector<std::shared_ptr<core::domain::TransactionDraft>> txDrafts;
    txDrafts.reserve(state.transactionDrafts.size());
    for (const auto& tx : state.transactionDrafts) {
        if (!tx) continue;
        if (tx->statementDraftId != out.id) continue;
        txDrafts.push_back(tx);
    }

    std::sort(txDrafts.begin(), txDrafts.end(), [](const auto& lhs, const auto& rhs) {
        if (!lhs) return false;
        if (!rhs) return true;
        return lhs->position < rhs->position;
    });

    out.transactions.reserve(txDrafts.size());
    for (const auto& tx : txDrafts) {
        if (!tx) continue;
        out.transactions.push_back(*tx);
    }

    return out;
}

void WorkspaceLogService::setImportLogs(core::domain::WorkspaceState& state, const std::vector<core::domain::ImportLog>& logs)
{
    state.importLogs.clear();
    state.importLogs.reserve(logs.size());
    for (const auto& item : logs) {
        auto log = std::make_shared<core::domain::ImportLog>(item);
        if (!log) continue;
        if (log->id.empty()) {
            log->id = core::utils::makeStableId();
        }
        state.importLogs.push_back(std::move(log));
    }
}

std::vector<std::shared_ptr<core::domain::ImportLog>> WorkspaceLogService::importLogs(const core::domain::WorkspaceState& state)
{
    return state.importLogs;
}

void WorkspaceLogService::setExportLogs(core::domain::WorkspaceState& state, const std::vector<core::domain::ExportLog>& logs)
{
    state.exportLogs.clear();
    state.exportLogs.reserve(logs.size());
    for (const auto& item : logs) {
        auto log = std::make_shared<core::domain::ExportLog>(item);
        if (!log) continue;
        if (log->id.empty()) {
            log->id = core::utils::makeStableId();
        }
        state.exportLogs.push_back(std::move(log));
    }
}

std::vector<std::shared_ptr<core::domain::ExportLog>> WorkspaceLogService::exportLogs(const core::domain::WorkspaceState& state)
{
    return state.exportLogs;
}

} // namespace core::application::workspace
