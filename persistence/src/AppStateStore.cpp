#include "persistence/AppStateStore.h"

#include <sqlite3.h>

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

// Deleted legacy numeric-id delete/remap helpers

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

    // NOTE: Removed authoritative "delete-not-in" behavior. Persisting now
    // performs upserts only and will not remove existing DB rows that are not
    // present in the provided AppState. This prevents accidental data loss
    // when temporary IDs or remapping are in-flight. Clients that need to
    // perform explicit deletions should call explicit delete APIs.

    // DB counts after save (log only)
    long long afterTx = db_count(db_->handle(), "SELECT COUNT(*) FROM transactions;");
    long long afterRel = db_count(db_->handle(), "SELECT COUNT(*) FROM transaction_properties;");
    fprintf(stderr, "DB after save: transactions=%lld transaction_properties=%lld\n", afterTx, afterRel);

    tx.commit();
    return result;
}
