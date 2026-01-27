/**
 * @file core/src/managers/AppStateManager.cpp
 * @brief Implementation of AppStateManager.
 *
 * Implements loading, saving, rehydration and validation of the application
 * state using the provided repository abstractions.
 */

#include "core/pch.h"

#include "core/managers/AppStateManager.h"

#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include "core/repositories/IActorRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"

#include <unordered_set>

AppStateManager::AppStateManager(Repositories repos) : repos_(std::move(repos)) {}

AppState AppStateManager::load() {
    AppState state;

    if (repos_.actors) state.actors = repos_.actors->getActors();
    if (repos_.properties) state.properties = repos_.properties->getProperties();
    if (repos_.contracts) state.contracts = repos_.contracts->getContracts();
    if (repos_.statements) state.statements = repos_.statements->getStatements();
    if (repos_.transactions) state.transactions = repos_.transactions->getTransactions();
    if (repos_.analyses) {
        auto vec = repos_.analyses->getAnalyses();
        state.analyses.clear();
        for (const auto& a : vec) state.analyses.push_back(a);
    }
    if (repos_.annuals) {
        auto vec = repos_.annuals->getAnnuals();
        state.annuals.clear();
        for (const auto& an : vec) state.annuals.push_back(an);
    }
    // annuals repository may be provided by persistence layer; load via dynamic_cast if available
    // to avoid breaking existing Repositories struct. If persistence provides an IAnnualRepository
    // add it to the Repositories and load similarly.
    if (repos_.analyses) state.analyses = repos_.analyses->getAnalyses();
    // load annuals if repository present (optional)
    // annuals repository may be provided by persistence layer
    // (ensure AppStateManager::Repositories contains it when constructed)
    // Note: we don't register an annuals field in Repositories struct to keep
    // compatibility; if needed, add it there.

    rehydrate(state);
    validate(state);

    return state;
}

void AppStateManager::save(const AppState& state) {
    validate(state);

    // Persist actors first

    if (repos_.actors) {
        for (const auto& a : state.actors) repos_.actors->upsertActor(a);
    }

    // Upsert properties. UI supplies stable UUID ids; repository will persist them.
    if (repos_.properties) {
        fprintf(stderr, "AppStateManager::save: properties before upsert:\n");
        for (const auto& p : state.properties) {
            if (!p) { fprintf(stderr, "  (null)\n"); continue; }
            fprintf(stderr, "  id='%s' name='%s'\n", p->id.c_str(), p->name.c_str());
        }
        for (const auto& p : state.properties) repos_.properties->upsertProperty(p);
    }

    // No property-id remapping: UI provides stable UUID ids for properties and
    // transactions reference those ids directly.

    if (repos_.contracts) {
        for (const auto& c : state.contracts) {
            if (!c) continue;
            c->actorIds.clear();
            for (auto* a : c->actors) { if (a && !a->id.empty()) c->actorIds.push_back(a->id); }
            c->propertyIds.clear();
            for (auto* p : c->properties) { if (p && !p->id.empty()) c->propertyIds.push_back(p->id); }
            repos_.contracts->upsertContract(c);
        }
    }

    if (repos_.statements) {
        // Persist statements only. Transactions are authoritative in the
        // global transactions list and will be persisted via the
        // transaction repository below.
        for (const auto& s : state.statements) {
            if (!s) continue;
            fprintf(stderr, "AppStateManager::save: upserting statement id='%s' name='%s'\n", s->id.c_str(), s->name.c_str());
            repos_.statements->upsertStatement(s);
        }
    }

    if (repos_.transactions) {
        for (const auto& t : state.transactions) {
            if (!t) continue;
            // Persist all transactions from the global transactions list.
            if (t->actor && !t->actor->id.empty()) t->actorId = t->actor->id;
            if (t->contract && !t->contract->id.empty()) t->contractId = t->contract->id;
            repos_.transactions->upsertTransaction(t);
        }
    }

    if (repos_.analyses) {
        for (const auto& a : state.analyses) if (a) repos_.analyses->upsertAnalysis(a);
    }

    if (repos_.annuals) {
        for (const auto& an : state.annuals) if (an) repos_.annuals->upsertAnnual(an);
    }
}

static void dedupStrings(std::vector<std::string>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

static void dedupContractRelations(Contract& c) {
    auto dedupPtr = [](auto& vec) {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    };
    dedupStrings(c.actorIds);
    dedupStrings(c.propertyIds);
    dedupPtr(c.actors);
    dedupPtr(c.properties);
}

void AppStateManager::rehydrate(AppState& state) {
    std::unordered_map<std::string, Actor*> actorById;
    actorById.reserve(state.actors.size());
    for (auto& a : state.actors) {
        if (!a || a->id.empty()) continue;
        actorById[a->id] = a.get();
    }

    std::unordered_map<std::string, Property*> propertyById;
    propertyById.reserve(state.properties.size());
    for (auto& p : state.properties) {
        if (!p || p->id.empty()) continue;
        propertyById[p->id] = p.get();
    }

    std::unordered_map<std::string, Contract*> contractById;
    contractById.reserve(state.contracts.size());
    for (auto& c : state.contracts) {
        if (!c || c->id.empty()) continue;
        contractById[c->id] = c.get();
        dedupContractRelations(*c);
    }

    std::unordered_map<std::string, Statement*> statementById;
    statementById.reserve(state.statements.size());
    for (auto& s : state.statements) {
        if (!s || s->id.empty()) continue;
        statementById[s->id] = s.get();
    }

    for (auto& cPtr : state.contracts) {
        if (!cPtr) continue;
        Contract& c = *cPtr;

        c.actors.clear();
        for (const auto& aid : c.actorIds) {
            auto it = actorById.find(aid);
            if (it != actorById.end()) c.actors.push_back(it->second);
        }

        c.properties.clear();
        for (const auto& pid : c.propertyIds) {
            auto it = propertyById.find(pid);
            if (it != propertyById.end()) c.properties.push_back(it->second);
        }

        dedupContractRelations(c);
    }

    for (auto& txPtr : state.transactions) {
        if (!txPtr) continue;
        Transaction& tx = *txPtr;

        if (!tx.actor && !tx.actorId.empty()) {
            auto it = actorById.find(tx.actorId);
            if (it != actorById.end()) tx.actor = it->second;
        }

        if (!tx.contract && !tx.contractId.empty()) {
            auto it = contractById.find(tx.contractId);
            if (it != contractById.end()) tx.contract = it->second;
        }
    }
}

void AppStateManager::validate(const AppState& state) const {
    auto fail = [&](const std::string& msg) {
        if (strictValidation_) throw std::runtime_error(msg);
    };

    for (const auto& c : state.contracts) {
        if (!c) continue;
        if (c->name.empty()) fail("Contract.name is empty");
        if (c->actorIds.empty() && c->actors.empty()) fail("Contract has no actors");
        if (c->propertyIds.empty() && c->properties.empty()) fail("Contract has no properties");
    }

    for (const auto& t : state.transactions) {
        if (!t) continue;
        if (t->name.empty()) fail("Transaction.name is empty");
        if (!t->actorId.empty() && t->actor == nullptr) fail("Transaction.actorId unresolved");
        if (!t->contractId.empty() && t->contract == nullptr) fail("Transaction.contractId unresolved");
    }
}
