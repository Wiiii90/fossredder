#include "persistence/AppStateStore.h"

#include <sqlite3.h>
#include <unordered_map>
#include <unordered_set>

#include "core/managers/AppStateManager.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "persistence/SqliteDb.h"
#include "persistence/SqliteTransaction.h"
#include "persistence/Factory.h"

#include <cstdio>

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

static std::unordered_set<long long> loadIds(sqlite3* db, const char* sql) {
    std::unordered_set<long long> out;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        out.insert(sqlite3_column_int64(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return out;
}

static std::vector<long long> loadContractIdsWhoseAllActorsAreDeleted(sqlite3* db, const std::unordered_set<long long>& keptActorIds, const std::unordered_set<long long>& keptContractIds) {
    std::vector<long long> toDelete;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT id FROM contracts;", -1, &stmt, nullptr) != SQLITE_OK) return toDelete;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long cid = sqlite3_column_int64(stmt, 0);
        if (keptContractIds.find(cid) != keptContractIds.end()) continue;

        sqlite3_stmt* rel = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT actor_id FROM contract_actors WHERE contract_id = ?;", -1, &rel, nullptr) != SQLITE_OK) continue;
        sqlite3_bind_int64(rel, 1, cid);

        bool hasAny = false;
        bool allMissing = true;
        while (sqlite3_step(rel) == SQLITE_ROW) {
            hasAny = true;
            long long aid = sqlite3_column_int64(rel, 0);
            if (keptActorIds.find(aid) != keptActorIds.end()) {
                allMissing = false;
                break;
            }
        }
        sqlite3_finalize(rel);

        if (!hasAny || allMissing) toDelete.push_back(cid);
    }
    sqlite3_finalize(stmt);
    return toDelete;
}

static void deleteIdsNotIn(sqlite3* db, const char* table, const char* idCol, const std::unordered_set<long long>& keep, std::vector<std::string>* deletedOut = nullptr) {
    auto todos = loadIds(db, (std::string("SELECT ") + idCol + " FROM " + table + ";").c_str());

    sqlite3_stmt* stmt = nullptr;
    std::string del = std::string("DELETE FROM ") + table + " WHERE " + idCol + " = ?;";
    if (sqlite3_prepare_v2(db, del.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return;

    for (auto id : todos) {
        if (keep.find(id) != keep.end()) continue;
        sqlite3_reset(stmt);
        sqlite3_bind_int64(stmt, 1, id);
        sqlite3_step(stmt);
        if (deletedOut) deletedOut->push_back(std::to_string(id));
    }

    sqlite3_finalize(stmt);
}

static void deleteIds(sqlite3* db, const char* table, const char* idCol, const std::vector<long long>& ids, std::vector<std::string>* deletedOut = nullptr) {
    sqlite3_stmt* stmt = nullptr;
    std::string del = std::string("DELETE FROM ") + table + " WHERE " + idCol + " = ?;";
    if (sqlite3_prepare_v2(db, del.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return;

    for (auto id : ids) {
        sqlite3_reset(stmt);
        sqlite3_bind_int64(stmt, 1, id);
        sqlite3_step(stmt);
        if (deletedOut) deletedOut->push_back(std::to_string(id));
    }

    sqlite3_finalize(stmt);
}

static long long db_count(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    long long out = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) out = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return out;
}

AppStateStore::AppStateStore(std::shared_ptr<SqliteDb> db) : db_(std::move(db)) {}

AppState AppStateStore::load() {
    auto actors = createSqliteActorRepository(db_);
    auto props = createSqlitePropertyRepository(db_);
    auto contracts = createSqliteContractRepository(db_);
    auto statements = createSqliteStatementRepository(db_);
    auto txRepo = createSqliteTransactionRepository(db_);

    AppStateManager::Repositories mgrRepos;
    mgrRepos.actors = actors;
    mgrRepos.properties = props;
    mgrRepos.contracts = contracts;
    mgrRepos.statements = statements;
    mgrRepos.transactions = txRepo;

    AppStateManager mgr(std::move(mgrRepos));
    AppState state = mgr.load();

    // Debug: report what was loaded from repositories
    fprintf(stderr, "AppStateStore::load: loaded - actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
            state.actors.size(), state.properties.size(), state.contracts.size(), state.statements.size(), state.transactions.size());
    for (size_t i = 0; i < state.statements.size(); ++i) {
        const auto& s = state.statements[i];
        if (!s) continue;
        fprintf(stderr, "  statement[%zu] id='%s' name='%s'\n", i, s->id.c_str(), s->name.c_str());
    }

    return state;
}

AppStateStoreResult AppStateStore::save(const AppState& state) {
    AppStateStoreResult result;

    auto actors = createSqliteActorRepository(db_);
    auto props = createSqlitePropertyRepository(db_);
    auto contracts = createSqliteContractRepository(db_);
    auto statements = createSqliteStatementRepository(db_);
    auto txRepo = createSqliteTransactionRepository(db_);

    SqliteTransaction tx(db_->handle());

    // Debug: print incoming state counts and sample transaction info
    fprintf(stderr, "AppStateStore::save: state - actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
            state.actors.size(), state.properties.size(), state.contracts.size(), state.statements.size(), state.transactions.size());

    for (size_t i = 0; i < state.transactions.size(); ++i) {
        const auto& t = state.transactions[i];
        if (!t) continue;
        fprintf(stderr, "  state.transaction[%zu] id='%s' stmt='%s' props=%zu status=%d alloc=%d\n",
                i, t->id.c_str(), t->statementId.c_str(), t->propertyIds.size(), static_cast<int>(t->status), t->allocatable ? 1 : 0);
        if (!t->propertyIds.empty()) {
            fprintf(stderr, "    propertyIds: ");
            for (const auto& pid : t->propertyIds) fprintf(stderr, "%s,", pid.c_str());
            fprintf(stderr, "\n");
        }
    }

    // DB counts before save
    long long beforeTx = db_count(db_->handle(), "SELECT COUNT(*) FROM transactions;");
    long long beforeRel = db_count(db_->handle(), "SELECT COUNT(*) FROM transaction_properties;");
    fprintf(stderr, "DB before save: transactions=%lld transaction_properties=%lld\n", beforeTx, beforeRel);

    // Persist current state (upserts).
    AppStateManager::Repositories mgrRepos;
    mgrRepos.actors = actors;
    mgrRepos.properties = props;
    mgrRepos.contracts = contracts;
    mgrRepos.statements = statements;
    mgrRepos.transactions = txRepo;

    AppStateManager mgr(std::move(mgrRepos));
    mgr.save(state);

    // After upserts, compute which DB IDs should be kept. Transactions are
    // authoritative in the global AppState::transactions list.
    std::unordered_set<long long> keepActors, keepProps, keepContracts, keepStatements, keepTx;
    for (const auto& a : state.actors) { if (a && !a->id.empty()) { auto id = toLL(a->id); if (id > 0) keepActors.insert(id); } }
    for (const auto& p : state.properties) { if (p && !p->id.empty()) { auto id = toLL(p->id); if (id > 0) keepProps.insert(id); } }
    for (const auto& c : state.contracts) { if (c && !c->id.empty()) { auto id = toLL(c->id); if (id > 0) keepContracts.insert(id); } }
    for (const auto& s : state.statements) { if (s && !s->id.empty()) { auto id = toLL(s->id); if (id > 0) keepStatements.insert(id); } }

    // global transactions list
    for (const auto& t : state.transactions) { if (t && !t->id.empty()) { auto id = toLL(t->id); if (id > 0) keepTx.insert(id); } }

    // Remove rows that are not present in the provided AppState. This implements
    // authoritative save semantics: the in-memory AppState is treated as ground
    // truth and dangling DB rows are removed. Deletions are recorded in
    // result.impact so callers can react (UI model updates).

    // Contracts that have no remaining actors should be removed as well.
    auto contractsToDelete = loadContractIdsWhoseAllActorsAreDeleted(db_->handle(), keepActors, keepContracts);
    if (!contractsToDelete.empty()) {
        deleteIds(db_->handle(), "contracts", "id", contractsToDelete, &result.impact.deletedContractIds);
    }

    // Transactions: remove DB rows whose id is not present in keepTx
    deleteIdsNotIn(db_->handle(), "transactions", "id", keepTx, &result.impact.deletedTransactionIds);

    // Statements, properties, actors: remove DB rows not present in keep sets
    deleteIdsNotIn(db_->handle(), "statements", "id", keepStatements, &result.impact.deletedStatementIds);
    deleteIdsNotIn(db_->handle(), "properties", "id", keepProps, &result.impact.deletedPropertyIds);
    deleteIdsNotIn(db_->handle(), "actors", "id", keepActors, &result.impact.deletedActorIds);

    // DB counts after save
    long long afterTx = db_count(db_->handle(), "SELECT COUNT(*) FROM transactions;");
    long long afterRel = db_count(db_->handle(), "SELECT COUNT(*) FROM transaction_properties;");
    fprintf(stderr, "DB after save: transactions=%lld transaction_properties=%lld\n", afterTx, afterRel);

    tx.commit();
    return result;
}
