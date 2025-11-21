#include "persistence/repositories/SqlitePropertyRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Property.h"

struct SqlitePropertyRepository::Impl { sqlite3* db = nullptr; };

SqlitePropertyRepository::SqlitePropertyRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql = "CREATE TABLE IF NOT EXISTS properties (name TEXT PRIMARY KEY, address TEXT, description TEXT);";
    char* err = nullptr; sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err); if (err) { sqlite3_free(err); }
}

SqlitePropertyRepository::~SqlitePropertyRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

void SqlitePropertyRepository::addProperty(const std::string& name, const std::string& address, const std::string& description) {
    const char* sql = "INSERT OR REPLACE INTO properties (name,address,description) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr; if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return; sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_text(stmt,2,address.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_text(stmt,3,description.c_str(),-1,SQLITE_TRANSIENT); sqlite3_step(stmt); sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Property>> SqlitePropertyRepository::getProperties() const {
    std::vector<std::shared_ptr<Property>> out; const char* sql = "SELECT name,address,description FROM properties ORDER BY name;"; sqlite3_stmt* stmt=nullptr; if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return out; while(sqlite3_step(stmt)==SQLITE_ROW){ const unsigned char* n=sqlite3_column_text(stmt,0); const unsigned char* a=sqlite3_column_text(stmt,1); const unsigned char* d=sqlite3_column_text(stmt,2); out.push_back(std::make_shared<Property>(std::string(reinterpret_cast<const char*>(n)), std::string(reinterpret_cast<const char*>(a? a:(const unsigned char*)"")), std::string(reinterpret_cast<const char*>(d? d:(const unsigned char*)"")))); } sqlite3_finalize(stmt); return out; }

void SqlitePropertyRepository::removeProperty(const std::string& name){ const char* sql="DELETE FROM properties WHERE name=?;"; sqlite3_stmt* stmt=nullptr; if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr)!=SQLITE_OK) return; sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT); sqlite3_step(stmt); sqlite3_finalize(stmt); }

void SqlitePropertyRepository::updateProperty(const std::string& name, const std::string& address, const std::string& description){ const char* sql="UPDATE properties SET address=?, description=? WHERE name=?;"; sqlite3_stmt* stmt=nullptr; if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return; sqlite3_bind_text(stmt,1,address.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_text(stmt,2,description.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_text(stmt,3,name.c_str(),-1,SQLITE_TRANSIENT); sqlite3_step(stmt); sqlite3_finalize(stmt); }
