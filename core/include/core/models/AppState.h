/**
 * @file core/include/core/models/AppState.h
 * @brief Aggregate in-memory application state.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core::domain {
class Property;
class Actor;
class Transaction;
class Statement;
class Contract;
class Analysis;
class Annual;

struct AppState {
    template <typename T>
    using EntityList = std::vector<std::shared_ptr<T>>;

    using PropertyList = EntityList<core::domain::Property>;
    using ActorList = EntityList<core::domain::Actor>;
    using ContractList = EntityList<core::domain::Contract>;
    using StatementList = EntityList<core::domain::Statement>;
    using TransactionList = EntityList<core::domain::Transaction>;
    using AnalysisList = EntityList<core::domain::Analysis>;
    using AnnualList = EntityList<core::domain::Annual>;

    PropertyList properties;
    ActorList actors;
    ContractList contracts;
    StatementList statements;
    TransactionList transactions;
    AnalysisList analyses;
    AnnualList annuals;

    [[nodiscard]] bool empty() const noexcept {
        return properties.empty() && actors.empty() && contracts.empty() && statements.empty() &&
               transactions.empty() && analyses.empty() && annuals.empty();
    }
};

}

using AppState = core::domain::AppState;