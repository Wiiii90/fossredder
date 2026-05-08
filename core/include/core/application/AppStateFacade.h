/**
 * @file core/include/core/application/AppStateFacade.h
 * @brief Declares the application-facing facade that owns editable app state and workspace persistence.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/application/CatalogService.h"
#include "core/errors/IErrorReporter.h"
#include "core/models/Alias.h"
#include "core/models/StatementDraft.h"
#include "core/models/ImportLog.h"
#include "core/models/ExportLog.h"
#include "core/models/Transaction.h"
#include "core/storage/IStorageManager.h"

namespace core::application {

class WorkspaceSession;

/**
 * @brief Exposes state mutation and workspace persistence as a single application facade.
 *
 * This type is consumed by presentation-layer controllers in `ui`, while the actual
 * MVC-style UI controllers remain outside `core`.
 */
class AppStateFacade {
public:
    using StateChanged = std::function<void(const core::domain::AppState&)>;

    /**
     * @brief Construct with unique ownership of an `IStorageManager`.
     * @param storageManager Unique pointer to the storage manager implementation.
     */
    explicit AppStateFacade(std::unique_ptr<core::storage::IStorageManager> storageManager);
    ~AppStateFacade();

    AppStateFacade(const AppStateFacade&) = delete;
    AppStateFacade& operator=(const AppStateFacade&) = delete;
    AppStateFacade(AppStateFacade&&) noexcept;
    AppStateFacade& operator=(AppStateFacade&&) noexcept;

    void setStateChangedCallback(StateChanged cb);
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter> reporter);
    void setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave saveFn);
    void setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad loadFn);
    void setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback cb);

    void openLatest();
    void newFile(const std::string& path);
    void openFile(const std::string& path);
    void saveFile();
    void saveFileAs(const std::string& path);

    const core::domain::AppState& state() const noexcept;
    const std::string& currentPath() const noexcept;

    std::string addActor(const std::string& name, const std::vector<core::domain::Alias>& aliases);
    void updateActor(const std::string& id, const std::string& name, const std::vector<core::domain::Alias>& aliases);
    void deleteActor(const std::string& id);

    std::string addProperty(const std::string& name, const std::vector<core::domain::Alias>& aliases);
    void updateProperty(const std::string& id, const std::string& name, const std::vector<core::domain::Alias>& aliases);
    void deleteProperty(const std::string& id);

    std::string addContract(const std::string& name, const std::string& type,
                            const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds,
                            const std::vector<core::domain::Alias>& aliases);
    void updateContract(const std::string& id, const std::string& name, const std::string& type,
                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds,
                        const std::vector<core::domain::Alias>& aliases);
    void deleteContract(const std::string& id);
    std::vector<std::string> contractTypes() const;

    std::string addStatement(const std::string& name);
    void updateStatement(const std::string& id, const std::string& name);
    void deleteStatement(const std::string& id);

    std::string addTransaction(const std::string& name,
                               const std::string& bookingDate,
                               double amount,
                               const std::string& statementId,
                               core::domain::Transaction::Status status,
                               const std::string& actorId,
                               bool allocatable,
                               const std::vector<std::string>& propertyIds);
    void updateTransaction(const std::string& id,
                           const std::string& name,
                           const std::string& bookingDate,
                           double amount,
                           const std::string& statementId,
                           core::domain::Transaction::Status status,
                           const std::string& actorId,
                           bool allocatable,
                           const std::vector<std::string>& propertyIds);
    void deleteTransaction(const std::string& id);

    std::string addAnalysis(const std::string& name,
                            const std::string& type,
                            const std::string& configJson,
                            const std::string& filterSpec,
                            const std::string& exportFormat,
                            bool includeCalcAdjustments,
                            const std::string& exportStateJson,
                            const std::string& snapshotTransactionsJson);
    void updateAnalysis(const std::string& id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& configJson,
                        const std::string& filterSpec,
                        const std::string& exportFormat,
                        bool includeCalcAdjustments,
                        const std::string& exportStateJson,
                        const std::string& snapshotTransactionsJson);
    void deleteAnalysis(const std::string& id);

    std::string addAnnual(const std::string& name,
                          int year,
                          const std::vector<std::string>& assignedAnalysisIds = {});
    void updateAnnual(const std::string& id,
                      const std::string& name,
                      int year,
                      const std::vector<std::string>& assignedAnalysisIds = {});
    void deleteAnnual(const std::string& id);

    std::string finalizeStatementDraft(const core::domain::StatementDraft& draft);
    void saveStatementDraft(const core::domain::StatementDraft& draft);
    void clearStatementDraft(const std::string& draftId = {});
    std::optional<core::domain::StatementDraft> loadStatementDraft(const std::string& draftId = {}) const;
    void setImportLogs(const std::vector<core::domain::ImportLog>& logs);
    std::vector<std::shared_ptr<core::domain::ImportLog>> importLogs() const;
    void setExportLogs(const std::vector<core::domain::ExportLog>& logs);
    std::vector<std::shared_ptr<core::domain::ExportLog>> exportLogs() const;
    void commit();
    void notifyState();

private:
    core::domain::AppState& mutableState() noexcept;

    CatalogService catalog_;
    std::unique_ptr<WorkspaceSession> session_;
};

} // namespace core::application
