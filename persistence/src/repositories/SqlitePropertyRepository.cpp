#include "persistence/repositories/SqlitePropertyRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Property.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqlitePropertyRepository::Impl {
    std::shared_ptr<SqliteDb> db;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath)
    : SqlitePropertyRepository(std::make_shared<SqliteDb>(dbPath), nullptr) {}

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath,
                                                   std::shared_ptr<core::errors::IErrorReporter> er)
    : SqlitePropertyRepository(std::make_shared<SqliteDb>(dbPath), std::move(er)) {}

SqlitePropertyRepository::SqlitePropertyRepository(std::shared_ptr<SqliteDb> db)
    : SqlitePropertyRepository(std::move(db), nullptr) {}

SqlitePropertyRepository::SqlitePropertyRepository(std::shared_ptr<SqliteDb> db,
                                                   std::shared_ptr<core::errors::IErrorReporter> er)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
    pimpl_->errorReporter = std::move(er);
}

SqlitePropertyRepository::~SqlitePropertyRepository() = default;

namespace {
constexpr auto kSelectProperty =
    "SELECT id, name, address, description, consumption, consumption_unit FROM properties";

std::shared_ptr<Property> readProperty(persistence::StmtGuard& s) {
    auto p = std::make_shared<Property>();
    p->id = s.columnText(0); p->name = s.columnText(1); p->address = s.columnText(2);
    p->description = s.columnText(3); p->consumption = s.columnDouble(4);
    p->consumptionUnit = s.columnText(5);
    return p;
}
} // namespace

void SqlitePropertyRepository::addProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO properties (id, name, address, description, consumption, consumption_unit)"
        " VALUES (?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText  (1, property->id);    stmt.bindText  (2, property->name);
    stmt.bindText  (3, property->address); stmt.bindText(4, property->description);
    stmt.bindDouble(5, property->consumption); stmt.bindText(6, property->consumptionUnit);
    stmt.step();
}

std::vector<std::shared_ptr<Property>> SqlitePropertyRepository::getProperties() const {
    std::vector<std::shared_ptr<Property>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectProperty) + " ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) out.push_back(readProperty(stmt));
    return out;
}

std::optional<std::shared_ptr<Property>> SqlitePropertyRepository::getPropertyById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        std::string(kSelectProperty) + " WHERE id = ?;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    return readProperty(stmt);
}

void SqlitePropertyRepository::removeProperty(const std::string& id) {
    if (id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM properties WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqlitePropertyRepository::updateProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE properties SET name=?, address=?, description=?, consumption=?, consumption_unit=?"
        " WHERE id=?;");
    if (!stmt) return;
    stmt.bindText  (1, property->name);    stmt.bindText  (2, property->address);
    stmt.bindText  (3, property->description); stmt.bindDouble(4, property->consumption);
    stmt.bindText  (5, property->consumptionUnit); stmt.bindText(6, property->id);
    stmt.step();
}

void SqlitePropertyRepository::upsertProperty(const std::shared_ptr<Property>& property) {
    if (!property || property->id.empty()) return;
    updateProperty(property);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addProperty(property);
}

void SqlitePropertyRepository::clearProperties() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM properties;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}


