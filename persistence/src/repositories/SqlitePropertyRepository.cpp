#include "persistence/repositories/SqlitePropertyRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Property.h"

struct SqlitePropertyRepository::Impl { sqlite3* db = nullptr; };

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS properties ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE,"
        "address TEXT,"
        "description TEXT,"
        "consumption REAL,"
        "consumption_unit TEXT"
        ");";
    char* err = nullptr;
    sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

SqlitePropertyRepository::~SqlitePropertyRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

void SqlitePropertyRepository::addProperty(const std::shared_ptr<Property>& property) {
    if (!property) return;

    const char* sql = "INSERT INTO properties (name, address, description, consumption, consumption_unit) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, property->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, property->address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, property->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, property->consumption);
    sqlite3_bind_text(stmt, 5, property->consumptionUnit.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        property->id = std::to_string(sqlite3_last_insert_rowid(pimpl_->db));
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Property>> SqlitePropertyRepository::getProperties() const {
    std::vector<std::shared_ptr<Property>> out;
    const char* sql = "SELECT id, name, address, description, consumption, consumption_unit FROM properties ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
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
        p->id = std::to_string(id);
        p->consumption = c;
        p->consumptionUnit = u ? reinterpret_cast<const char*>(u) : std::string();
        out.push_back(std::move(p));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Property>> SqlitePropertyRepository::getPropertyById(const std::string& id) const {
    long long pid = toLL(id);
    if (pid <= 0) return std::nullopt;

    const char* sql = "SELECT id, name, address, description, consumption, consumption_unit FROM properties WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, pid);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    long long rid = sqlite3_column_int64(stmt, 0);
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
    p->id = std::to_string(rid);
    p->consumption = c;
    p->consumptionUnit = u ? reinterpret_cast<const char*>(u) : std::string();

    sqlite3_finalize(stmt);
    return p;
}

void SqlitePropertyRepository::removeProperty(const std::string& id){
    long long pid = toLL(id);
    if (pid <= 0) return;

    const char* sql = "DELETE FROM properties WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, pid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqlitePropertyRepository::updateProperty(const std::shared_ptr<Property>& property){
    if (!property) return;
    long long pid = toLL(property->id);
    if (pid <= 0) return;

    const char* sql = "UPDATE properties SET name = ?, address = ?, description = ?, consumption = ?, consumption_unit = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, property->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, property->address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, property->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, property->consumption);
    sqlite3_bind_text(stmt, 5, property->consumptionUnit.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, pid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
