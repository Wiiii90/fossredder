/**
 * @file persistence/include/persistence/repositories/SqliteTransactionDraftRepository.h
 * @brief Declares the SQLite-backed transaction draft repository.
 */

#pragma once

#include "core/repositories/ITransactionDraftRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteTransactionDraftRepository : public ITransactionDraftRepository {
public:
    explicit SqliteTransactionDraftRepository(const std::string& dbPath);
    explicit SqliteTransactionDraftRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteTransactionDraftRepository() override;

    void addTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) override;
    std::vector<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDrafts() const override;
    std::optional<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDraftById(const std::string& id) const override;
    void removeTransactionDraft(const std::string& id) override;
    void updateTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) override;
    void upsertTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) override;
    void clearTransactionDrafts() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
