#include "core/pch.h"
#include "core/managers/AppStateManager.h"

#include <stdexcept>
#include <unordered_map>

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

AppStateManager::AppStateManager(Repositories repos) : repos_(std::move(repos)) {}

AppState AppStateManager::load() {
    AppState state;

    if (repos_.actors) state.actors = repos_.actors->getActors();
    if (repos_.properties) state.properties = repos_.properties->getProperties();
    if (repos_.contracts) state.contracts = repos_.contracts->getContracts();
    if (repos_.statements) state.statements = repos_.statements->getStatements();
    if (repos_.transactions) state.transactions = repos_.transactions->getTransactions();

    rehydrate(state);
    validate(state);

    return state;
}

void AppStateManager::save(const AppState& state) {
    if (repos_.actors) {
        for (const auto& a : state.actors) repos_.actors->upsertActor(a);
    }

    if (repos_.properties) {
        for (const auto& p : state.properties) repos_.properties->upsertProperty(p);
    }

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
        for (const auto& s : state.statements) repos_.statements->upsertStatement(s);
    }

    if (repos_.transactions) {
        for (const auto& t : state.transactions) {
            if (!t) continue;
            if (t->actor && !t->actor->id.empty()) t->actorId = t->actor->id;
            if (t->contract && !t->contract->id.empty()) t->contractId = t->contract->id;
            repos_.transactions->upsertTransaction(t);
        }
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

        if (!tx.statementId.empty()) {
            auto it = statementById.find(tx.statementId);
            if (it != statementById.end()) {
                Statement& st = *it->second;
                auto existing = std::find(st.transactions.begin(), st.transactions.end(), tx);
                if (existing == st.transactions.end()) {
                    st.transactions.push_back(tx);
                }
            }
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
