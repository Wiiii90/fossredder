#include "persistence/repositories/SqliteAnnualRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Annual.h"
#include "persistence/SqliteDb.h"
#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <stdexcept>

namespace {

std::string serializeIds(const std::vector<std::string>& v)
{
    return nlohmann::json(v).dump();
}

std::vector<std::string> deserializeIds(const std::string& s)
{
    if (s.empty()) return {};
    const auto parsed = nlohmann::json::parse(s, nullptr, false);
    if (!parsed.is_array()) return {};
    return parsed.get<std::vector<std::string>>();
}

constexpr auto kSelectAnnual =
    "SELECT id, name, year, transaction_ids, assigned_analysis_ids, verification_state,"
    " created_at, updated_at, schema_version FROM annuals";

std::shared_ptr<Annual> readAnnual(persistence::StmtGuard& s) {
    auto a = std::make_shared<Annual>();
    a->id = s.columnText(0);
    a->name = s.columnText(1);
    a->year = s.columnInt(2);
    a->transactionIds       = deserializeIds(s.columnText(3));
    a->assignedAnalysisIds  = deserializeIds(s.columnText(4));
    a->verificationState    = static_cast<Annual::VerificationState>(s.columnInt(5));
    a->createdAt            = s.columnText(6);
    a->updatedAt            = s.columnText(7);
    a->schemaVersion        = s.columnInt(8);
    return a;
}

} // namespace

struct SqliteAnnualRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteAnnualRepository::SqliteAnnualRepository(const std::string& dbPath)
    : SqliteAnnualRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteAnnualRepository::SqliteAnnualRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteAnnualRepository::~SqliteAnnualRepository() = default;

void SqliteAnnualRepository::addAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual || annual->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO annuals"
        " (id, name, year, transaction_ids, assigned_analysis_ids, verification_state,"
        "  created_at, updated_at, schema_version)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, annual->id);
    stmt.bindText(2, annual->name);
    stmt.bindInt (3, annual->year);
    stmt.bindText(4, serializeIds(annual->transactionIds));
    stmt.bindText(5, serializeIds(annual->assignedAnalysisIds));
    stmt.bindInt (6, static_cast<int>(annual->verificationState));
    stmt.bindText(7, annual->createdAt);
    stmt.bindText(8, annual->updatedAt);
    stmt.bindInt (9, annual->schemaVersion);
    stmt.step();
}

std::vector<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnuals() const {
    std::vector<std::shared_ptr<Annual>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectAnnual) + " ORDER BY year;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) out.push_back(readAnnual(stmt));
    return out;
}

std::optional<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnualById(const std::string& id) const {
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectAnnual) + " WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    return readAnnual(stmt);
}

void SqliteAnnualRepository::removeAnnual(const std::string& id) {
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM annuals WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteAnnualRepository::updateAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual || annual->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE annuals SET name=?, year=?, transaction_ids=?, assigned_analysis_ids=?,"
        " verification_state=?, created_at=?, updated_at=?, schema_version=? WHERE id=?;");
    if (!stmt) return;
    stmt.bindText(1, annual->name);
    stmt.bindInt (2, annual->year);
    stmt.bindText(3, serializeIds(annual->transactionIds));
    stmt.bindText(4, serializeIds(annual->assignedAnalysisIds));
    stmt.bindInt (5, static_cast<int>(annual->verificationState));
    stmt.bindText(6, annual->createdAt);
    stmt.bindText(7, annual->updatedAt);
    stmt.bindInt (8, annual->schemaVersion);
    stmt.bindText(9, annual->id);
    stmt.step();
}

void SqliteAnnualRepository::upsertAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual || annual->id.empty()) return;
    updateAnnual(annual);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addAnnual(annual);
}

void SqliteAnnualRepository::clearAnnuals() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM annuals;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteAnnualRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
