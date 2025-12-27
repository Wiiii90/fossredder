#include "persistence/SqliteSchema.h"

#include <sqlite3.h>
#include <stdexcept>
#include <string>

static void exec(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        if (err) sqlite3_free(err);
        throw std::runtime_error(e);
    }
}

int SqliteSchema::getUserVersion(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) != SQLITE_OK) return 0;
    int v = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) v = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return v;
}

void SqliteSchema::setUserVersion(sqlite3* db, int v) {
    exec(db, (std::string("PRAGMA user_version = ") + std::to_string(v) + ";").c_str());
}

void SqliteSchema::ensure(sqlite3* db) {
    if (!db) throw std::runtime_error("db is null");
    exec(db, "PRAGMA foreign_keys = ON;");
    migrate(db);
}

void SqliteSchema::migrate(sqlite3* db) {
    int v = getUserVersion(db);

    if (v < 1) {
        exec(db,
            "BEGIN;"
            "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY KEY, value TEXT);"
            "CREATE TABLE IF NOT EXISTS actors ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT UNIQUE,"
            "type TEXT,"
            "description TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS properties ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT UNIQUE,"
            "address TEXT,"
            "description TEXT,"
            "consumption REAL,"
            "consumption_unit TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS contracts ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT,"
            "type TEXT,"
            "description TEXT,"
            "start_date TEXT,"
            "end_date TEXT,"
            "base_price REAL,"
            "consumption_price REAL,"
            "monthly_advance REAL"
            ");"
            "CREATE TABLE IF NOT EXISTS contract_actors ("
            "contract_id INTEGER NOT NULL,"
            "actor_id INTEGER NOT NULL,"
            "PRIMARY KEY(contract_id, actor_id),"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
            "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS contract_properties ("
            "contract_id INTEGER NOT NULL,"
            "property_id INTEGER NOT NULL,"
            "PRIMARY KEY(contract_id, property_id),"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
            "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS statements ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT,"
            "start_date TEXT,"
            "end_date TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS transactions ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT,"
            "booking_date TEXT,"
            "valuta TEXT,"
            "amount REAL,"
            "description TEXT,"
            "actor_id INTEGER,"
            "contract_id INTEGER,"
            "statement_id INTEGER,"
            "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE SET NULL,"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE SET NULL,"
            "FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE"
            ");"
            "COMMIT;"
        );
        setUserVersion(db, 1);
        v = 1;
    }
}
