#include "persistence/AppStateStore.h"

#include <sqlite3.h>

#include "core/application/AppStateManager.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "persistence/SqliteDb.h"
#include "persistence/SqliteTransaction.h"
#include "persistence/Factory.h"

#include <cstdio>
#include <unordered_set>

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
    auto repos = createSqliteRepositoryBundle(db_);
    core::application::AppStateManager mgr(std::move(repos));
    AppState state = mgr.load();

    

    return state;
}

AppStateStoreResult AppStateStore::save(const AppState& state) {
    AppStateStoreResult result;

    auto repos = createSqliteRepositoryBundle(db_);
    auto txRepo = repos.transactions;

    SqliteTransaction tx(db_->handle());

    

    // DB counts before save
    long long beforeTx = db_count(db_->handle(), "SELECT COUNT(*) FROM transactions;");
    long long beforeRel = db_count(db_->handle(), "SELECT COUNT(*) FROM transaction_properties;");
    

    // Persist current state (upserts).
    core::application::AppStateManager mgr(std::move(repos));
    mgr.save(state);

    auto collectActorIds = [&](const auto& vec) {
        std::unordered_set<std::string> ids;
        ids.reserve(vec.size());
        for (const auto& item : vec) {
            if (!item || item->id.empty()) continue;
            ids.insert(item->id);
        }
        return ids;
    };

    const auto actorIds = collectActorIds(state.actors);
    const auto propertyIds = collectActorIds(state.properties);
    const auto contractIds = collectActorIds(state.contracts);
    const auto statementIds = collectActorIds(state.statements);
    const auto transactionIds = collectActorIds(state.transactions);
    const auto analysisIds = collectActorIds(state.analyses);
    const auto annualIds = collectActorIds(state.annuals);

    if (txRepo) {
        for (const auto& t : txRepo->getTransactions()) {
            if (!t || t->id.empty()) continue;
            if (transactionIds.find(t->id) != transactionIds.end()) continue;
            txRepo->removeTransaction(t->id);
            result.impact.deletedTransactionIds.push_back(t->id);
        }
    }

    if (repos.statements) {
        for (const auto& s : repos.statements->getStatements()) {
            if (!s || s->id.empty()) continue;
            if (statementIds.find(s->id) != statementIds.end()) continue;
            repos.statements->removeStatement(s->id);
            result.impact.deletedStatementIds.push_back(s->id);
        }
    }

    if (repos.contracts) {
        for (const auto& c : repos.contracts->getContracts()) {
            if (!c || c->id.empty()) continue;
            if (contractIds.find(c->id) != contractIds.end()) continue;
            repos.contracts->removeContract(c->id);
            result.impact.deletedContractIds.push_back(c->id);
        }
    }

    if (repos.actors) {
        for (const auto& a : repos.actors->getActors()) {
            if (!a || a->id.empty()) continue;
            if (actorIds.find(a->id) != actorIds.end()) continue;
            repos.actors->removeActor(a->id);
            result.impact.deletedActorIds.push_back(a->id);
        }
    }

    if (repos.properties) {
        for (const auto& p : repos.properties->getProperties()) {
            if (!p || p->id.empty()) continue;
            if (propertyIds.find(p->id) != propertyIds.end()) continue;
            repos.properties->removeProperty(p->id);
            result.impact.deletedPropertyIds.push_back(p->id);
        }
    }

    if (repos.analyses) {
        for (const auto& a : repos.analyses->getAnalyses()) {
            if (!a || a->id.empty()) continue;
            if (analysisIds.find(a->id) != analysisIds.end()) continue;
            repos.analyses->removeAnalysis(a->id);
        }
    }

    if (repos.annuals) {
        for (const auto& an : repos.annuals->getAnnuals()) {
            if (!an || an->id.empty()) continue;
            if (annualIds.find(an->id) != annualIds.end()) continue;
            repos.annuals->removeAnnual(an->id);
        }
    }

    

    tx.commit();
    return result;
}
