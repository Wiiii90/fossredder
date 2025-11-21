#include "persistence/repositories/SqliteConfigRepository.h"
#include "core/models/Config.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqliteConfigRepository::Impl { sqlite3* db = nullptr; };

SqliteConfigRepository::SqliteConfigRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql = "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY KEY, value TEXT);";
    char* err = nullptr; if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); }
}

SqliteConfigRepository::~SqliteConfigRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

bool SqliteConfigRepository::saveConfig(const std::string& name, const std::shared_ptr<Config>& config) {
    const char* sql = "INSERT OR REPLACE INTO configs (name, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
    std::string s = config->languageToString();
    sqlite3_bind_text(stmt,2,s.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return true;
}

std::optional<std::shared_ptr<Config>> SqliteConfigRepository::loadConfig(const std::string& name) const {
    const char* sql = "SELECT value FROM configs WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }
    const unsigned char* v = sqlite3_column_text(stmt,0);
    std::string s = v ? reinterpret_cast<const char*>(v) : std::string();
    sqlite3_finalize(stmt);
    auto cfg = std::make_shared<Config>();
    cfg->language = Config::languageFromString(s);
    return cfg;
}

std::vector<std::string> SqliteConfigRepository::listConfigs() const {
    std::vector<std::string> out;
    const char* sql = "SELECT name FROM configs ORDER BY name;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return out;
    while(sqlite3_step(stmt)==SQLITE_ROW){ const unsigned char* n=sqlite3_column_text(stmt,0); out.push_back(std::string(reinterpret_cast<const char*>(n))); }
    sqlite3_finalize(stmt);
    return out;
}

bool SqliteConfigRepository::deleteConfig(const std::string& name){
    const char* sql="DELETE FROM configs WHERE name=?;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return false;
    sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return true;
}

void SqliteConfigRepository::setDefaultConfig(const std::string& name){
    const char* sql = "INSERT OR REPLACE INTO configs (name, value) VALUES ('__default__', ?);";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<std::shared_ptr<Config>> SqliteConfigRepository::getDefaultConfig() const {
    const char* sql = "SELECT value FROM configs WHERE name='__default__';";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return std::nullopt;
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }
    const unsigned char* v = sqlite3_column_text(stmt,0);
    std::string name = v? reinterpret_cast<const char*>(v) : std::string();
    sqlite3_finalize(stmt);
    if (name.empty()) return std::nullopt;
    return loadConfig(name);
}

std::string SqliteConfigRepository::getDefaultConfigName() const {
    const char* sql = "SELECT value FROM configs WHERE name='__default__';";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return std::string();
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::string(); }
    const unsigned char* v = sqlite3_column_text(stmt,0);
    std::string s = v? reinterpret_cast<const char*>(v) : std::string();
    sqlite3_finalize(stmt);
    return s;
}
