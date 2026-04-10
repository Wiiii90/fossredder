/**
 * @file persistence/src/repositories/SqliteStatementDraftRepository.cpp
 * @brief Implements the SQLite-backed statement draft repository.
 */

#include "persistence/repositories/SqliteStatementDraftRepository.h"

#include "core/models/StatementDraft.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

void writeStatementDraft(sqlite3* db,
                         const std::shared_ptr<core::domain::StatementDraft>& draft,
                         const char* sql)
{
    if (!db || !draft) {
        return;
    }

    if (draft->id.empty()) {
        draft->id = persistence::generateUuid();
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return;
    }

    stmt.bindText(1, draft->id);
    stmt.bindText(2, draft->name);
    stmt.step();
}

} // namespace

struct SqliteStatementDraftRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteStatementDraftRepository::SqliteStatementDraftRepository(const std::string& dbPath)
    : SqliteStatementDraftRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteStatementDraftRepository::SqliteStatementDraftRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteStatementDraftRepository::~SqliteStatementDraftRepository() = default;

void SqliteStatementDraftRepository::addStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft)
{
    writeStatementDraft(
        pimpl_->db->handle(),
        draft,
        "INSERT INTO statement_drafts (id, name) VALUES (?, ?);");
}

std::vector<std::shared_ptr<core::domain::StatementDraft>> SqliteStatementDraftRepository::getStatementDrafts() const
{
    std::vector<std::shared_ptr<core::domain::StatementDraft>> drafts;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "SELECT id, name FROM statement_drafts ORDER BY id;");
    if (!stmt) {
        return drafts;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto draft = std::make_shared<core::domain::StatementDraft>();
        draft->id = stmt.columnText(0);
        draft->name = stmt.columnText(1);
        drafts.push_back(std::move(draft));
    }

    return drafts;
}

std::optional<std::shared_ptr<core::domain::StatementDraft>> SqliteStatementDraftRepository::getStatementDraftById(const std::string& id) const
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "SELECT id, name FROM statement_drafts WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto draft = std::make_shared<core::domain::StatementDraft>();
    draft->id = stmt.columnText(0);
    draft->name = stmt.columnText(1);
    return draft;
}

void SqliteStatementDraftRepository::removeStatementDraft(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statement_drafts WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteStatementDraftRepository::updateStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft)
{
    writeStatementDraft(
        pimpl_->db->handle(),
        draft,
        "UPDATE statement_drafts SET name = ?2 WHERE id = ?1;");
}

void SqliteStatementDraftRepository::upsertStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft)
{
    writeStatementDraft(
        pimpl_->db->handle(),
        draft,
        "INSERT OR REPLACE INTO statement_drafts (id, name) VALUES (?, ?);");
}

void SqliteStatementDraftRepository::clearStatementDrafts()
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statement_drafts;");
    if (!stmt) {
        return;
    }
    stmt.step();
}
