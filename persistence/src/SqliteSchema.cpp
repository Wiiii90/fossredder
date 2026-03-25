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
        // Initial schema: use TEXT primary keys to allow UUIDs as ids
        exec(db,
            "BEGIN;"
            "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY KEY, value TEXT);"
            "CREATE TABLE IF NOT EXISTS actors ("
            "id TEXT PRIMARY KEY,"
            "name TEXT UNIQUE,"
            "type TEXT,"
            "description TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS properties ("
            "id TEXT PRIMARY KEY,"
            "name TEXT UNIQUE,"
            "address TEXT,"
            "description TEXT,"
            "consumption REAL,"
            "consumption_unit TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS contracts ("
            "id TEXT PRIMARY KEY,"
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
            "contract_id TEXT NOT NULL,"
            "actor_id TEXT NOT NULL,"
            "PRIMARY KEY(contract_id, actor_id),"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
            "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS contract_properties ("
            "contract_id TEXT NOT NULL,"
            "property_id TEXT NOT NULL,"
            "PRIMARY KEY(contract_id, property_id),"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
            "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS statements ("
            "id TEXT PRIMARY KEY,"
            "name TEXT"
            ");"
            "CREATE TABLE IF NOT EXISTS transactions ("
            "id TEXT PRIMARY KEY,"
            "name TEXT,"
            "booking_date TEXT,"
            "valuta TEXT,"
            "amount REAL,"
            "status INTEGER NOT NULL DEFAULT 0,"
            "description TEXT,"
            "type TEXT,"
            "actor_id TEXT,"
            "contract_id TEXT,"
            "statement_id TEXT,"
            "metadata TEXT,"
            "proof_image_path TEXT,"
            "allocatable INTEGER NOT NULL DEFAULT 0,"
            "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE SET NULL,"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE SET NULL,"
            "FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS transaction_properties ("
            "transaction_id TEXT NOT NULL,"
            "property_id TEXT NOT NULL,"
            "PRIMARY KEY(transaction_id, property_id),"
            "FOREIGN KEY(transaction_id) REFERENCES transactions(id) ON DELETE CASCADE,"
            "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS analyses ("
            "id TEXT PRIMARY KEY,"
            "name TEXT,"
            "type TEXT,"
            "config_json TEXT,"
            "filter_spec TEXT,"
            "created_at TEXT,"
            "updated_at TEXT,"
            "schema_version INTEGER"
            ");"
            "CREATE TABLE IF NOT EXISTS annuals ("
            "id TEXT PRIMARY KEY,"
            "year INTEGER,"
            "transaction_ids TEXT,"
            "assigned_analysis_ids TEXT,"
            "verification_state INTEGER NOT NULL DEFAULT 0,"
            "created_at TEXT,"
            "updated_at TEXT,"
            "schema_version INTEGER"
            ");"
            "CREATE INDEX IF NOT EXISTS idx_transactions_contract_id ON transactions(contract_id);"
            "CREATE INDEX IF NOT EXISTS idx_contract_actors_actor_id ON contract_actors(actor_id);"
            "CREATE INDEX IF NOT EXISTS idx_contract_properties_property_id ON contract_properties(property_id);"
            "CREATE INDEX IF NOT EXISTS idx_transaction_properties_property_id ON transaction_properties(property_id);"
            "COMMIT;"
        );
        setUserVersion(db, 1);
        v = 1;
    }

    if (v < 2) {
        exec(db,
            "BEGIN;"
            "CREATE TABLE IF NOT EXISTS actor_aliases ("
            "actor_id TEXT NOT NULL,"
            "alias TEXT NOT NULL,"
            "hit_count INTEGER NOT NULL DEFAULT 1,"
            "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "PRIMARY KEY(actor_id, alias),"
            "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS property_aliases ("
            "property_id TEXT NOT NULL,"
            "alias TEXT NOT NULL,"
            "hit_count INTEGER NOT NULL DEFAULT 1,"
            "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "PRIMARY KEY(property_id, alias),"
            "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
            ");"
            "CREATE TABLE IF NOT EXISTS contract_aliases ("
            "contract_id TEXT NOT NULL,"
            "alias TEXT NOT NULL,"
            "hit_count INTEGER NOT NULL DEFAULT 1,"
            "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "PRIMARY KEY(contract_id, alias),"
            "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE"
            ");"
            "CREATE INDEX IF NOT EXISTS idx_actor_aliases_alias ON actor_aliases(alias);"
            "CREATE INDEX IF NOT EXISTS idx_property_aliases_alias ON property_aliases(alias);"
            "CREATE INDEX IF NOT EXISTS idx_contract_aliases_alias ON contract_aliases(alias);"
            "COMMIT;"
        );
        setUserVersion(db, 2);
        v = 2;
    }
}
