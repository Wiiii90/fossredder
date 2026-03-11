/**
 * @file core/src/application/AppStateManager.cpp
 * @brief Implementation of AppStateManager.
 *
 * Implements loading, saving, rehydration and validation of the application
 * state using the provided repository abstractions.
 */

#include "core/pch.h"

#include "core/application/AppStateManager.h"
#include "core/application/StateHydrator.h"
#include "core/application/StateProjector.h"

#include "core/repositories/IActorRepository.h"
#include "core/repositories/IAnalysisRepository.h"
#include "core/repositories/IAnnualRepository.h"
#include "core/repositories/IContractRepository.h"
#include "core/repositories/IPropertyRepository.h"
#include "core/repositories/IStatementRepository.h"
#include "core/repositories/ITransactionRepository.h"

#include "core/models/Property.h"
#include "core/models/Statement.h"

namespace core::application {

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

    StateHydrator::rehydrate(state);
    StateHydrator::validate(state, strictValidation_);

    return state;
}

void AppStateManager::save(const AppState& state) {
    AppState projected = StateProjector::prepareForSave(state);
    StateHydrator::validate(projected, strictValidation_);

    if (repos_.actors) {
        for (const auto& a : projected.actors) {
            if (!a) continue;
            repos_.actors->upsertActor(a);
        }
    }

    if (repos_.properties) {
        for (const auto& p : projected.properties) {
            if (!p) continue;
            repos_.properties->upsertProperty(p);
        }
    }

    if (repos_.contracts) {
        for (const auto& c : projected.contracts) {
            if (!c) continue;
            repos_.contracts->upsertContract(c);
        }
    }

    if (repos_.statements) {
        for (const auto& s : projected.statements) {
            if (!s) continue;
            repos_.statements->upsertStatement(s);
        }
    }

    if (repos_.transactions) {
        for (const auto& t : projected.transactions) {
            if (!t) continue;
            repos_.transactions->upsertTransaction(t);
        }
    }

    if (repos_.analyses) {
        for (const auto& a : projected.analyses) if (a) repos_.analyses->upsertAnalysis(a);
    }

    if (repos_.annuals) {
        for (const auto& an : projected.annuals) if (an) repos_.annuals->upsertAnnual(an);
    }
}

}
