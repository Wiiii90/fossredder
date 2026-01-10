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

#include <unordered_set>

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
    validate(state);

    // Record temporary id -> Property shared_ptr for properties that may have UI-generated IDs
    std::unordered_map<std::string, std::shared_ptr<Property>> tempIdToPtr;
    tempIdToPtr.reserve(state.properties.size());
    for (const auto& p : state.properties) {
        if (!p) continue;
        if (!p->id.empty()) tempIdToPtr[p->id] = p;
    }

    fprintf(stderr, "AppStateManager::save: tempIdToPtr keys:\n");
    for (const auto& kv : tempIdToPtr) fprintf(stderr, "  tempId=%s name=%s\n", kv.first.c_str(), kv.second ? kv.second->name.c_str() : "(null)");

    if (repos_.actors) {
        for (const auto& a : state.actors) repos_.actors->upsertActor(a);
    }

    // helper to parse numeric id (defined early so subsequent code can use it)
    auto parseIdNum = [](const std::string& s) -> long long {
        try { size_t pos = 0; long long v = std::stoll(s, &pos); if (pos == s.size()) return v; } catch (...) {}
        return -1;
    };

    // Upsert properties first so they receive final numeric IDs
    if (repos_.properties) {
        // Log property ids/names before upsert
        fprintf(stderr, "AppStateManager::save: properties before upsert:\n");
        for (const auto& p : state.properties) {
            if (!p) { fprintf(stderr, "  (null)\n"); continue; }
            fprintf(stderr, "  id='%s' name='%s'\n", p->id.c_str(), p->name.c_str());
        }

        for (const auto& p : state.properties) repos_.properties->upsertProperty(p);

        // Reload persisted properties and update in-memory property ids by name
        try {
            auto persisted = repos_.properties->getProperties();
            std::unordered_map<std::string, std::string> persistedNameToId;
            for (const auto& pp : persisted) {
                if (!pp) continue;
                persistedNameToId[pp->name] = pp->id;
            }
            for (const auto& p : state.properties) {
                if (!p) continue;
                // if id is non-numeric, try to replace by persisted id via name
                if (parseIdNum(p->id) <= 0) {
                    auto it = persistedNameToId.find(p->name);
                    if (it != persistedNameToId.end()) p->id = it->second;
                }
            }

            // Log persisted properties and mapping
            fprintf(stderr, "AppStateManager::save: persisted properties (reloaded):\n");
            for (const auto& pp : persisted) {
                if (!pp) { fprintf(stderr, "  (null)\n"); continue; }
                fprintf(stderr, "  id='%s' name='%s'\n", pp->id.c_str(), pp->name.c_str());
            }

            fprintf(stderr, "AppStateManager::save: properties after remap:\n");
            for (const auto& p : state.properties) {
                if (!p) { fprintf(stderr, "  (null)\n"); continue; }
                fprintf(stderr, "  id='%s' name='%s' (numeric? %s)\n", p->id.c_str(), p->name.c_str(), (parseIdNum(p->id) > 0) ? "yes" : "no");
            }

            // If any properties still have non-numeric ids, attempt to insert them explicitly
            bool anyNonNumeric = false;
            for (const auto& p : state.properties) {
                if (!p) continue;
                if (parseIdNum(p->id) <= 0) { anyNonNumeric = true; break; }
            }
            if (anyNonNumeric) {
                fprintf(stderr, "AppStateManager::save: Attempting explicit insert for non-numeric properties...\n");
                for (const auto& p : state.properties) {
                    if (!p) continue;
                    if (parseIdNum(p->id) > 0) continue;
                    try {
                        repos_.properties->addProperty(p);
                        fprintf(stderr, "AppStateManager::save: addProperty attempted for name='%s' -> id now='%s'\n", p->name.c_str(), p->id.c_str());
                    } catch (...) {
                        fprintf(stderr, "AppStateManager::save: addProperty threw for name='%s'\n", p->name.c_str());
                    }
                }

                // reload persisted and remap again
                try {
                    auto persisted2 = repos_.properties->getProperties();
                    std::unordered_map<std::string, std::string> persistedNameToId2;
                    for (const auto& pp : persisted2) {
                        if (!pp) continue;
                        persistedNameToId2[pp->name] = pp->id;
                    }
                    for (const auto& p : state.properties) {
                        if (!p) continue;
                        if (parseIdNum(p->id) <= 0) {
                            auto it = persistedNameToId2.find(p->name);
                            if (it != persistedNameToId2.end()) p->id = it->second;
                        }
                    }

                    fprintf(stderr, "AppStateManager::save: persisted properties (reloaded after explicit insert):\n");
                    for (const auto& pp : persisted2) {
                        if (!pp) { fprintf(stderr, "  (null)\n"); continue; }
                        fprintf(stderr, "  id='%s' name='%s'\n", pp->id.c_str(), pp->name.c_str());
                    }

                    fprintf(stderr, "AppStateManager::save: properties after second remap:\n");
                    for (const auto& p : state.properties) {
                        if (!p) { fprintf(stderr, "  (null)\n"); continue; }
                        fprintf(stderr, "  id='%s' name='%s' (numeric? %s)\n", p->id.c_str(), p->name.c_str(), (parseIdNum(p->id) > 0) ? "yes" : "no");
                    }
                } catch (...) {
                    // ignore
                }
            }
        } catch (...) {
            // ignore reload failures; remapping will log unresolved ids
        }
    }

    // Build mapping from temporary id -> final numeric id using the same shared_ptrs
    std::unordered_map<std::string, std::string> tempToFinalId;
    tempToFinalId.reserve(tempIdToPtr.size());
    for (const auto& kv : tempIdToPtr) {
        const auto& tempId = kv.first;
        const auto& ptr = kv.second;
        if (!ptr) continue;
        if (!ptr->id.empty()) tempToFinalId[tempId] = ptr->id;
    }

    fprintf(stderr, "AppStateManager::save: tempToFinalId map:\n");
    for (const auto& kv : tempToFinalId) fprintf(stderr, "  %s -> %s\n", kv.first.c_str(), kv.second.c_str());

    // Build fallback map name -> id in case transactions hold property names instead of ids
    std::unordered_map<std::string, std::string> nameToId;
    nameToId.reserve(state.properties.size());
    for (const auto& p : state.properties) {
        if (!p) continue;
        if (!p->name.empty() && !p->id.empty()) nameToId[p->name] = p->id;
    }

    fprintf(stderr, "AppStateManager::save: nameToId map:\n");
    for (const auto& kv : nameToId) fprintf(stderr, "  %s -> %s\n", kv.first.c_str(), kv.second.c_str());

    // replace temporary property ids in-place using tempId->finalId map or fallback to name->id
    auto remapPropertyIds = [&](std::vector<std::string>& propIds) {
        for (auto& pid : propIds) {
            if (pid.empty()) continue;
            if (parseIdNum(pid) > 0) {
                fprintf(stderr, "AppStateManager::save: pid '%s' already numeric\n", pid.c_str());
                continue; // already numeric
            }
            fprintf(stderr, "AppStateManager::save: resolving pid '%s'...\n", pid.c_str());
            auto it = tempToFinalId.find(pid);
            if (it != tempToFinalId.end()) {
                fprintf(stderr, "  resolved via tempToFinalId -> %s\n", it->second.c_str());
                pid = it->second; continue;
            }
            auto itn = nameToId.find(pid);
            if (itn != nameToId.end()) {
                fprintf(stderr, "  resolved via nameToId -> %s\n", itn->second.c_str());
                pid = itn->second; continue;
            }
            // unresolved -> log for diagnostics
            fprintf(stderr, "AppStateManager::save: unresolved property id '%s'\n", pid.c_str());
        }
    };

    // Remap for global transactions
    for (const auto& t : state.transactions) {
        if (!t) continue;
        remapPropertyIds(t->propertyIds);
    }

    // Property id remapping is performed for global transactions in state.transactions.

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
