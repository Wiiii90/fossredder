/**
 * @file persistence/include/persistence/repositories/SqliteStatementDraftRepository.h
 * @brief Declares the SQLite-backed statement draft repository.
 */

#pragma once

#include "core/repositories/IStatementDraftRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteStatementDraftRepository : public IStatementDraftRepository {
public:
    explicit SqliteStatementDraftRepository(const std::string& dbPath);
    explicit SqliteStatementDraftRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteStatementDraftRepository() override;

    void addStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) override;
    std::vector<std::shared_ptr<core::domain::StatementDraft>> getStatementDrafts() const override;
    std::optional<std::shared_ptr<core::domain::StatementDraft>> getStatementDraftById(const std::string& id) const override;
    void removeStatementDraft(const std::string& id) override;
    void updateStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) override;
    void upsertStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) override;
    void clearStatementDrafts() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
