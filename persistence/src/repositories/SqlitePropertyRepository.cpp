#include "persistence/repositories/SqlitePropertyRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Property.h"
#include "persistence/SqliteDb.h"
#include "persistence/Uuid.h"

struct SqlitePropertyRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath)
    : SqlitePropertyRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqlitePropertyRepository::SqlitePropertyRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqlitePropertyRepository::~SqlitePropertyRepository() = default;

void SqlitePropertyRepository::addProperty(const std::shared_ptr<Property>& property) {
    if (!property) return;
    // Ensure property has a string id (UUID) before insert
    if (property->id.empty()) property->id = persistence::generateUuid();

    const char* sql = "INSERT INTO properties (id, name, address, description, consumption, consumption_unit) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, property->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, property->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, property->address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, property->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, property->consumption);
    sqlite3_bind_text(stmt, 6, property->consumptionUnit.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        fprintf(stderr, "SqlitePropertyRepository::addProperty: inserted property id='%s' name='%s'\n", property->id.c_str(), property->name.c_str());
    } else {
        // Insertion failed (e.g., UNIQUE constraint). Try to find existing property by name
        const char* err = sqlite3_errmsg(pimpl_->db->handle());
        fprintf(stderr, "SqlitePropertyRepository::addProperty: insert failed for name='%s': %s\n", property->name.c_str(), err ? err : "unknown");
        sqlite3_finalize(stmt);
        const char* sel = "SELECT id FROM properties WHERE name = ? LIMIT 1;";
        sqlite3_stmt* selStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), sel, -1, &selStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(selStmt, 1, property->name.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(selStmt) == SQLITE_ROW) {
                const unsigned char* rid = sqlite3_column_text(selStmt, 0);
                if (rid) {
                    property->id = reinterpret_cast<const char*>(rid);
                    fprintf(stderr, "SqlitePropertyRepository::addProperty: found existing property id='%s' for name='%s'\n", property->id.c_str(), property->name.c_str());
                }
            }
            sqlite3_finalize(selStmt);
        }
        return;
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Property>> SqlitePropertyRepository::getProperties() const {
    std::vector<std::shared_ptr<Property>> out;
    const char* sql = "SELECT id, name, address, description, consumption, consumption_unit FROM properties ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* n = sqlite3_column_text(stmt, 1);
        const unsigned char* a = sqlite3_column_text(stmt, 2);
        const unsigned char* d = sqlite3_column_text(stmt, 3);
        double c = sqlite3_column_double(stmt, 4);
        const unsigned char* u = sqlite3_column_text(stmt, 5);

        auto p = std::make_shared<Property>(
            n ? reinterpret_cast<const char*>(n) : std::string(),
            a ? reinterpret_cast<const char*>(a) : std::string(),
            d ? reinterpret_cast<const char*>(d) : std::string()
        );
        p->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        p->consumption = c;
        p->consumptionUnit = u ? reinterpret_cast<const char*>(u) : std::string();
        out.push_back(std::move(p));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Property>> SqlitePropertyRepository::getPropertyById(const std::string& id) const {
    if (id.empty()) return std::nullopt;

    const char* sql = "SELECT id, name, address, description, consumption, consumption_unit FROM properties WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    const unsigned char* rid = sqlite3_column_text(stmt, 0);
    const unsigned char* n = sqlite3_column_text(stmt, 1);
    const unsigned char* a = sqlite3_column_text(stmt, 2);
    const unsigned char* d = sqlite3_column_text(stmt, 3);
    double c = sqlite3_column_double(stmt, 4);
    const unsigned char* u = sqlite3_column_text(stmt, 5);

    auto p = std::make_shared<Property>(
        n ? reinterpret_cast<const char*>(n) : std::string(),
        a ? reinterpret_cast<const char*>(a) : std::string(),
        d ? reinterpret_cast<const char*>(d) : std::string()
    );
    p->id = rid ? reinterpret_cast<const char*>(rid) : std::string();
    p->consumption = c;
    p->consumptionUnit = u ? reinterpret_cast<const char*>(u) : std::string();

    sqlite3_finalize(stmt);
    return p;
}

void SqlitePropertyRepository::removeProperty(const std::string& id){
    if (id.empty()) return;

    const char* sql = "DELETE FROM properties WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqlitePropertyRepository::updateProperty(const std::shared_ptr<Property>& property){
    if (!property) return;
    if (property->id.empty()) return;

    const char* sql = "UPDATE properties SET name = ?, address = ?, description = ?, consumption = ?, consumption_unit = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, property->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, property->address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, property->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, property->consumption);
    sqlite3_bind_text(stmt, 5, property->consumptionUnit.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, property->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    fprintf(stderr, "SqlitePropertyRepository::updateProperty: updated id='%s' name='%s'\n", property->id.c_str(), property->name.c_str());
}

void SqlitePropertyRepository::upsertProperty(const std::shared_ptr<Property>& property) {
    if (!property) return;
    if (property->id.empty()) { addProperty(property); return; }
    updateProperty(property);
    if (pimpl_ && pimpl_->db && sqlite3_changes(pimpl_->db->handle()) == 0) {
        addProperty(property);
    }
}

void SqlitePropertyRepository::clearProperties() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM properties;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
