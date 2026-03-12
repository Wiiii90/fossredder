#include "persistence/repositories/SqliteContractRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Contract.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqliteContractRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteContractRepository::SqliteContractRepository(const std::string& dbPath)
    : SqliteContractRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteContractRepository::SqliteContractRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteContractRepository::~SqliteContractRepository() = default;

namespace {

void insertContractRelations(sqlite3* db, const std::string& contractId, const Contract& c) {
    {
        persistence::StmtGuard s(db,
            "INSERT OR IGNORE INTO contract_actors (contract_id, actor_id) VALUES (?, ?);");
        if (s) for (const auto& id : c.actorIds) {
            if (id.empty()) continue;
            s.reset(); s.bindText(1, contractId); s.bindText(2, id); s.step();
        }
    }
    {
        persistence::StmtGuard s(db,
            "INSERT OR IGNORE INTO contract_properties (contract_id, property_id) VALUES (?, ?);");
        if (s) for (const auto& id : c.propertyIds) {
            if (id.empty()) continue;
            s.reset(); s.bindText(1, contractId); s.bindText(2, id); s.step();
        }
    }
}

void deleteContractRelations(sqlite3* db, const std::string& contractId) {
    persistence::StmtGuard a(db, "DELETE FROM contract_actors WHERE contract_id = ?;");
    if (a) { a.bindText(1, contractId); a.step(); }
    persistence::StmtGuard p(db, "DELETE FROM contract_properties WHERE contract_id = ?;");
    if (p) { p.bindText(1, contractId); p.step(); }
}

constexpr auto kSelectContract =
    "SELECT id, name, type, description, start_date, end_date, base_price,"
    " consumption_price, monthly_advance FROM contracts";

std::shared_ptr<Contract> readContractRow(persistence::StmtGuard& s) {
    auto c = std::make_shared<Contract>();
    c->id = s.columnText(0); c->name = s.columnText(1); c->type = s.columnText(2);
    c->description = s.columnText(3); c->startDate = s.columnText(4);
    c->endDate = s.columnText(5); c->basePrice = s.columnDouble(6);
    c->consumptionPrice = s.columnDouble(7); c->monthlyAdvance = s.columnDouble(8);
    return c;
}

void loadContractRelations(sqlite3* db, Contract& c) {
    {
        persistence::StmtGuard s(db,
            "SELECT actor_id FROM contract_actors WHERE contract_id = ?;");
        if (s) { s.bindText(1, c.id); while (s.step() == SQLITE_ROW) c.actorIds.push_back(s.columnText(0)); }
    }
    {
        persistence::StmtGuard s(db,
            "SELECT property_id FROM contract_properties WHERE contract_id = ?;");
        if (s) { s.bindText(1, c.id); while (s.step() == SQLITE_ROW) c.propertyIds.push_back(s.columnText(0)); }
    }
}

} // namespace

void SqliteContractRepository::addContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO contracts"
        " (id, name, type, description, start_date, end_date, base_price, consumption_price, monthly_advance)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, contract->id);    stmt.bindText(2, contract->name);
    stmt.bindText(3, contract->type);  stmt.bindText(4, contract->description);
    stmt.bindText(5, contract->startDate); stmt.bindText(6, contract->endDate);
    stmt.bindDouble(7, contract->basePrice); stmt.bindDouble(8, contract->consumptionPrice);
    stmt.bindDouble(9, contract->monthlyAdvance);
    if (stmt.step() == SQLITE_DONE)
        insertContractRelations(pimpl_->db->handle(), contract->id, *contract);
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContracts() const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectContract) + " ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto c = readContractRow(stmt);
        loadContractRelations(pimpl_->db->handle(), *c);
        out.push_back(std::move(c));
    }
    return out;
}

std::optional<std::shared_ptr<Contract>> SqliteContractRepository::getContractById(const std::string& id) const {
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectContract) + " WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto c = readContractRow(stmt);
    loadContractRelations(pimpl_->db->handle(), *c);
    return c;
}

void SqliteContractRepository::removeContract(const std::string& id) {
    deleteContractRelations(pimpl_->db->handle(), id);
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM contracts WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteContractRepository::updateContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE contracts SET name=?, type=?, description=?, start_date=?, end_date=?,"
        " base_price=?, consumption_price=?, monthly_advance=? WHERE id=?;");
    if (!stmt) return;
    stmt.bindText(1, contract->name);    stmt.bindText(2, contract->type);
    stmt.bindText(3, contract->description); stmt.bindText(4, contract->startDate);
    stmt.bindText(5, contract->endDate); stmt.bindDouble(6, contract->basePrice);
    stmt.bindDouble(7, contract->consumptionPrice); stmt.bindDouble(8, contract->monthlyAdvance);
    stmt.bindText(9, contract->id);
    stmt.step();
    deleteContractRelations(pimpl_->db->handle(), contract->id);
    insertContractRelations(pimpl_->db->handle(), contract->id, *contract);
}

void SqliteContractRepository::upsertContract(const std::shared_ptr<Contract>& contract) {
    if (!contract || contract->id.empty()) return;
    updateContract(contract);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addContract(contract);
}

void SqliteContractRepository::clearContracts() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(),
        "DELETE FROM contract_actors; DELETE FROM contract_properties; DELETE FROM contracts;",
        nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForActor(const std::string& actorId) const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard ids(pimpl_->db->handle(),
        "SELECT contract_id FROM contract_actors WHERE actor_id = ?;");
    if (!ids) return out;
    ids.bindText(1, actorId);
    while (ids.step() == SQLITE_ROW) {
        auto c = getContractById(ids.columnText(0));
        if (c) out.push_back(*c);
    }
    return out;
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForProperty(const std::string& propertyId) const {
    std::vector<std::shared_ptr<Contract>> out;
    persistence::StmtGuard ids(pimpl_->db->handle(),
        "SELECT contract_id FROM contract_properties WHERE property_id = ?;");
    if (!ids) return out;
    ids.bindText(1, propertyId);
    while (ids.step() == SQLITE_ROW) {
        auto c = getContractById(ids.columnText(0));
        if (c) out.push_back(*c);
    }
    return out;
}

std::vector<std::string> SqliteContractRepository::getActorIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT actor_id FROM contract_actors WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
    while (stmt.step() == SQLITE_ROW) out.push_back(stmt.columnText(0));
    return out;
}

std::vector<std::string> SqliteContractRepository::getPropertyIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT property_id FROM contract_properties WHERE contract_id = ?;");
    if (!stmt) return out;
    stmt.bindText(1, contractId);
    while (stmt.step() == SQLITE_ROW) out.push_back(stmt.columnText(0));
    return out;
}
