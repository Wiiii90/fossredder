/**
 * @file persistence/src/repositories/SqliteTransactionDraftRepository.cpp
 * @brief Implements the SQLite-backed transaction draft repository.
 */

#include "persistence/repositories/SqliteTransactionDraftRepository.h"

#include "core/models/TransactionDraft.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

void bindNullableText(persistence::StmtGuard& stmt, int column, const std::string& value)
{
    if (value.empty()) {
        sqlite3_bind_null(stmt.get(), column);
        return;
    }
    stmt.bindText(column, value);
}

int nextTransactionDraftPosition(sqlite3* db)
{
    if (!db) return 0;
    persistence::StmtGuard stmt(db, "SELECT COALESCE(MAX(position), -1) + 1 FROM transaction_drafts;");
    if (!stmt) return 0;
    if (stmt.step() != SQLITE_ROW) return 0;
    return stmt.columnInt(0);
}

void removeTransactionDraftProperties(sqlite3* db, const std::string& id)
{
    persistence::StmtGuard stmt(db, "DELETE FROM transaction_draft_properties WHERE transaction_draft_id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id);
    stmt.step();
}

void upsertTransactionDraftProperties(sqlite3* db, const core::domain::TransactionDraft& draft)
{
    persistence::StmtGuard stmt(
        db,
        "INSERT OR REPLACE INTO transaction_draft_properties (transaction_draft_id, property_position, property_id) VALUES (?, ?, ?);");
    if (!stmt) return;

    for (std::size_t propertyIndex = 0; propertyIndex < draft.propertyIds.size(); ++propertyIndex) {
        const auto& propertyId = draft.propertyIds[propertyIndex];
        if (propertyId.empty()) continue;
        stmt.reset();
        stmt.bindText(1, draft.id);
        stmt.bindInt(2, static_cast<int>(propertyIndex));
        stmt.bindText(3, propertyId);
        stmt.step();
    }
}

void writeTransactionDraft(sqlite3* db,
                           const std::shared_ptr<core::domain::TransactionDraft>& draft,
                           const char* sql,
                           bool autoPosition)
{
    if (!db || !draft) return;
    if (draft->id.empty()) {
        draft->id = persistence::generateUuid();
    }
    if (autoPosition && draft->position <= 0) {
        draft->position = nextTransactionDraftPosition(db);
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) return;

    stmt.bindText(1, draft->id);
    bindNullableText(stmt, 2, draft->statementDraftId);
    stmt.bindInt(3, draft->position);
    stmt.bindText(4, draft->name);
    stmt.bindText(5, draft->bookingDate);
    stmt.bindText(6, draft->valuta);
    stmt.bindDouble(7, draft->amount);
    stmt.bindText(8, draft->description);
    stmt.bindText(9, draft->actorText);
    stmt.bindText(10, draft->propertyText);
    bindNullableText(stmt, 11, draft->actorId);
    stmt.bindInt(12, draft->newActorSelected ? 1 : 0);
    bindNullableText(stmt, 13, draft->contractId);
    stmt.bindInt(14, draft->newContractSelected ? 1 : 0);
    stmt.bindText(15, draft->metadata);
    stmt.bindText(16, draft->proofImagePath);
    stmt.bindText(17, draft->type);
    stmt.bindInt(18, draft->allocatable ? 1 : 0);
    stmt.bindInt(19, draft->allocatableManualOverride ? 1 : 0);
    stmt.bindInt(20, static_cast<int>(draft->status));
    stmt.step();

    removeTransactionDraftProperties(db, draft->id);
    upsertTransactionDraftProperties(db, *draft);
}

std::shared_ptr<core::domain::TransactionDraft> readDraftRow(sqlite3* db, persistence::StmtGuard& stmt)
{
    auto draft = std::make_shared<core::domain::TransactionDraft>();
    draft->id = stmt.columnText(0);
    draft->statementDraftId = stmt.columnText(1);
    draft->position = stmt.columnInt(2);
    draft->name = stmt.columnText(3);
    draft->bookingDate = stmt.columnText(4);
    draft->valuta = stmt.columnText(5);
    draft->amount = stmt.columnDouble(6);
    draft->description = stmt.columnText(7);
    draft->actorText = stmt.columnText(8);
    draft->propertyText = stmt.columnText(9);
    draft->actorId = stmt.columnText(10);
    draft->newActorSelected = stmt.columnInt(11) != 0;
    draft->contractId = stmt.columnText(12);
    draft->newContractSelected = stmt.columnInt(13) != 0;
    draft->metadata = stmt.columnText(14);
    draft->proofImagePath = stmt.columnText(15);
    draft->type = stmt.columnText(16);
    draft->allocatable = stmt.columnInt(17) != 0;
    draft->allocatableManualOverride = stmt.columnInt(18) != 0;
    draft->status = static_cast<core::domain::Transaction::Status>(stmt.columnInt(19));

    persistence::StmtGuard properties(
        db,
        "SELECT property_id FROM transaction_draft_properties WHERE transaction_draft_id = ? ORDER BY property_position;");
    if (properties) {
        properties.bindText(1, draft->id);
        while (properties.step() == SQLITE_ROW) {
            draft->propertyIds.push_back(properties.columnText(0));
        }
    }

    return draft;
}

} // namespace

struct SqliteTransactionDraftRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteTransactionDraftRepository::SqliteTransactionDraftRepository(const std::string& dbPath)
    : SqliteTransactionDraftRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteTransactionDraftRepository::SqliteTransactionDraftRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteTransactionDraftRepository::~SqliteTransactionDraftRepository() = default;

void SqliteTransactionDraftRepository::addTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft)
{
    writeTransactionDraft(
        pimpl_->db->handle(),
        draft,
        "INSERT INTO transaction_drafts (id, statement_draft_id, position, name, booking_date, valuta, amount, description, actor_text, property_text, actor_id, new_actor_selected, contract_id, new_contract_selected, metadata, proof_image_path, type, allocatable, allocatable_manual_override, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        true);
}

std::vector<std::shared_ptr<core::domain::TransactionDraft>> SqliteTransactionDraftRepository::getTransactionDrafts() const
{
    std::vector<std::shared_ptr<core::domain::TransactionDraft>> drafts;

    persistence::StmtGuard stmt(
        pimpl_->db->handle(),
        "SELECT id, statement_draft_id, position, name, booking_date, valuta, amount, description, actor_text, property_text, actor_id, new_actor_selected, contract_id, new_contract_selected, metadata, proof_image_path, type, allocatable, allocatable_manual_override, status FROM transaction_drafts ORDER BY statement_draft_id, position, id;");
    if (!stmt) {
        return drafts;
    }

    while (stmt.step() == SQLITE_ROW) {
        drafts.push_back(readDraftRow(pimpl_->db->handle(), stmt));
    }

    return drafts;
}

std::optional<std::shared_ptr<core::domain::TransactionDraft>> SqliteTransactionDraftRepository::getTransactionDraftById(const std::string& id) const
{
    persistence::StmtGuard stmt(
        pimpl_->db->handle(),
        "SELECT id, statement_draft_id, position, name, booking_date, valuta, amount, description, actor_text, property_text, actor_id, new_actor_selected, contract_id, new_contract_selected, metadata, proof_image_path, type, allocatable, allocatable_manual_override, status FROM transaction_drafts WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    return readDraftRow(pimpl_->db->handle(), stmt);
}

void SqliteTransactionDraftRepository::removeTransactionDraft(const std::string& id)
{
    removeTransactionDraftProperties(pimpl_->db->handle(), id);

    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM transaction_drafts WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteTransactionDraftRepository::updateTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft)
{
    writeTransactionDraft(
        pimpl_->db->handle(),
        draft,
        "UPDATE transaction_drafts SET statement_draft_id = ?2, position = ?3, name = ?4, booking_date = ?5, valuta = ?6, amount = ?7, description = ?8, actor_text = ?9, property_text = ?10, actor_id = ?11, new_actor_selected = ?12, contract_id = ?13, new_contract_selected = ?14, metadata = ?15, proof_image_path = ?16, type = ?17, allocatable = ?18, allocatable_manual_override = ?19, status = ?20 WHERE id = ?1;",
        false);
}

void SqliteTransactionDraftRepository::upsertTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft)
{
    writeTransactionDraft(
        pimpl_->db->handle(),
        draft,
        "INSERT OR REPLACE INTO transaction_drafts (id, statement_draft_id, position, name, booking_date, valuta, amount, description, actor_text, property_text, actor_id, new_actor_selected, contract_id, new_contract_selected, metadata, proof_image_path, type, allocatable, allocatable_manual_override, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        true);
}

void SqliteTransactionDraftRepository::clearTransactionDrafts()
{
    persistence::StmtGuard deleteProperties(pimpl_->db->handle(), "DELETE FROM transaction_draft_properties;");
    if (deleteProperties) {
        deleteProperties.step();
    }

    persistence::StmtGuard deleteDrafts(pimpl_->db->handle(), "DELETE FROM transaction_drafts;");
    if (deleteDrafts) {
        deleteDrafts.step();
    }
}
