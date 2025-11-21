#include "persistence/repositories/SqliteBookingGroupRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/BookingGroup.h"

struct SqliteBookingGroupRepository::Impl { sqlite3* db = nullptr; };

SqliteBookingGroupRepository::SqliteBookingGroupRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql = "CREATE TABLE IF NOT EXISTS booking_groups (id INTEGER PRIMARY KEY AUTOINCREMENT, statement_id INTEGER NOT NULL, title TEXT, FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE);";
    char* err = nullptr; if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); }
}

SqliteBookingGroupRepository::~SqliteBookingGroupRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

void SqliteBookingGroupRepository::addBookingGroup(const std::string& statementId, const std::shared_ptr<BookingGroup>& group) {
    const char* sql = "INSERT INTO booking_groups (statement_id, title) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,statementId.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,group->title.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<BookingGroup>> SqliteBookingGroupRepository::getBookingGroupsForStatement(const std::string& statementId) const {
    std::vector<std::shared_ptr<BookingGroup>> out;
    const char* sql = "SELECT id, title FROM booking_groups WHERE statement_id = ? ORDER BY id;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db,sql,-1,&stmt,nullptr)!=SQLITE_OK) return out;
    sqlite3_bind_text(stmt,1,statementId.c_str(),-1,SQLITE_TRANSIENT);
    while(sqlite3_step(stmt)==SQLITE_ROW){
        long long id=sqlite3_column_int64(stmt,0);
        const unsigned char* t=sqlite3_column_text(stmt,1);
        auto bg = std::make_shared<BookingGroup>();
        bg->title = t? reinterpret_cast<const char*>(t) : std::string();
        out.push_back(bg);
    }
    sqlite3_finalize(stmt);
    return out;
}

void SqliteBookingGroupRepository::removeBookingGroup(const std::string& id){
    const char* sql = "DELETE FROM booking_groups WHERE id = ?;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql,-1,&stmt,nullptr)!=SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteBookingGroupRepository::updateBookingGroup(const std::string& id, const std::shared_ptr<BookingGroup>& group){
    const char* sql = "UPDATE booking_groups SET title = ? WHERE id = ?;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,group->title.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
